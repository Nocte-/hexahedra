//---------------------------------------------------------------------------
// client/game.cpp
//
// This file is part of Hexahedra.
//
// Hexahedra is free software; you can redistribute it and/or modify it
// under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012-2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "main_game.hpp"

#include <iomanip>
#include <iostream>
#include <unordered_set>

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <SFML/Graphics/Image.hpp>

#include <es/component.hpp>
#include <es/entity.hpp>

#include <hexa/algorithm.hpp>
#include <hexa/collision.hpp>
#include <hexa/compression.hpp>
#include <hexa/chunk.hpp>
#include <hexa/config.hpp>
#include <hexa/geometric.hpp>
#include <hexa/log.hpp>
#include <hexa/os.hpp>
#include <hexa/protocol.hpp>
#include <hexa/lightmap.hpp>
#include <hexa/ray.hpp>
#include <hexa/voxel_algorithm.hpp>
#include <hexa/process.hpp>
#include <hexa/trace.hpp>
#include <hexa/utf8.hpp>
#include <hexa/voxel_range.hpp>

#include "clock.hpp"
#include "event.hpp"
#include "player_info.hpp"
#include "renderer_i.hpp"
#include "sfml_ogl2.hpp"
#include "sfml_ogl3.hpp"
#include "../persistence_null.hpp"
#include "../persistence_leveldb.hpp"
#include "../entity_system.hpp"
#include "../entity_system_physics.hpp"

#include "loading_screen.hpp"

using namespace boost;
using namespace boost::math::float_constants;
namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace pt = boost::property_tree;

using boost::format;

namespace hexa {

extern po::variables_map global_settings;

main_game::main_game (game& the_game, const std::string& host, uint16_t port,
                      unsigned int vd)
    : game_state     (the_game)
    , udp_client     (host, port)
    , old_chunk_pos_ (0, 0, 0)
    , old_position_  (0, 0, 0)
    , old_fraction_  (0, 0, 0)
    , scene_         (*this)
    , stop_          (false)
    , in_action_     (false)
    , asio_          ([=]{io_.run();})
    , player_entity_ (0xffffffff)
    , waiting_for_data_(true)
    , loading_screen_(false)
    , singleplayer_  (host.empty())
    , show_ui_       (true)
    , ignore_text_   (0)
{
    if (singleplayer_)
    {
        try
        {
            log_msg("Launching server...");
            fs::path dir (executable_path().parent_path());
            log_msg("Path is %1%", dir);
            server_process_ = start_process(dir / "hexahedra-server", { "--mode=singleplayer" });
            log_msg("Done.");
            boost::this_thread::sleep_for(boost::chrono::milliseconds(300));
        }
        catch (std::runtime_error& e)
        {
            log_msg("Cannot launch server (%1%), attempt to connect to multiplayer localhost",
                    e.what());

            singleplayer_ = false;
        }
    }

    game_.relative_mouse(true);
    setup_renderer();
    setup_world(host, port);
    scene_.view_distance(vd);

    log_msg("Trying to connect to %1%:%2% ...", host, port);
    int tries (0);
    while (!connect())
    {
        log_msg("   ... retrying...");
        ++tries;
        if (tries > 2)
        {
            if (singleplayer_)
                terminate_process(server_process_);

            throw std::runtime_error("cannot connect to server");
        }
    }
    log_msg("Connected!");
    login();
    log_msg("Logged in successfully");
}

main_game::~main_game()
{
    clock_.join();

    if (singleplayer_)
        terminate_process(server_process_);
}

void main_game::setup_world (const std::string& host, uint16_t port)
{
    fs::path user_dir (global_settings["userdir"].as<std::string>());
    fs::path data_dir (global_settings["datadir"].as<std::string>());

    fs::path gameroot (user_dir / "games");
    std::string host_id ((format("%1%.%2%") % host % port).str());
    fs::path gamepath (gameroot / host_id);

    if (   !fs::exists(gamepath)
        && !fs::create_directories(gamepath))
    {
        throw std::runtime_error((format("error: cannot create directory '%1%'") % gamepath.string()).str());
    }

    storage_ = std::make_unique<persistence_leveldb>(gamepath / "world.leveldb");
    map_     = std::make_unique<chunk_cache>(*storage_);
}

void main_game::setup_renderer()
{
    // Pick a renderer based on the OpenGL driver and the program settings.

    extern po::variables_map global_settings;

    std::string ogl_version ((const char*)glGetString(GL_VERSION));
    log_msg("OpenGL version: %1%", ogl_version);
    int ogl_major (ogl_version[0] - '0'), ogl_minor (ogl_version[2] - '0');
    if (ogl_major == 1 && ogl_minor < 5)
    {
        throw std::runtime_error("Sorry, you need at least OpenGL 1.5");
    }

    if (   (ogl_major == 1 && ogl_minor >= 5) || ogl_major == 2
        || global_settings.count("ogl2"))
    {
        // OpenGL 1.5 or 2.x
        renderer_ = std::make_unique<sfml_ogl2>(window(), scene_);
    }
    else
    {
        // OpenGL 3 or newer
        renderer_ = std::make_unique<sfml_ogl3>(window(), scene_);
    }
}

void main_game::set_view_distance(unsigned int d)
{
    scene_.view_distance(d);
}

void main_game::resize (unsigned int w, unsigned int h)
{
    trace("window resize %1%x%2%", w, h);
    renderer().resize(w, h);
}

void main_game::player_controls()
{
    if (hud_.show_input())
        return;

    if (key_pressed(key::space))
        action(0);

    static msg::motion last;
    static msg::look_at last_look;

    msg::motion mesg;

    // Translate the key press status of the WSAD keys to an angle
    // and a speed, both encoded in 8 bits.

    int y (1);
    if (key_pressed(key::w)) ++y;
    if (key_pressed(key::s)) --y;

    int x (1);
    if (key_pressed(key::d)) ++x;
    if (key_pressed(key::a)) --x;

    static const std::array<uint8_t, 9> dirs {{ 0xA0, 0x80, 0x60, 0xC0, 0x00, 0x40, 0xE0, 0x00, 0x20 }};
    int index (x + y * 3);

    mesg.position = player_.get_wfpos();
    mesg.move_dir = dirs[index];
    mesg.move_speed = (index == 4 ? 0x00 : 0xff);

    if (mesg.move_dir != last.move_dir || mesg.move_speed != last.move_speed )
    {
        send(serialize_packet(mesg), mesg.method());
        last = mesg;

        if (player_entity_ != 0xffffffff)
            walk(float(mesg.move_dir) / 256.f * two_pi,
                 float(mesg.move_speed) / 256.f);
    }

    msg::look_at look (player_.head_angle());
    if (look.look != last_look.look)
    {
        send(serialize_packet(look), look.method());
        last_look = look;

        if (player_entity_ != 0xffffffff)
        {
            auto lock (entities_.acquire_write_lock());
            entities_.set(player_entity_, entity_system::c_lookat, look.look);
        }
    }
}

void main_game::update(double time_delta)
{
    if (waiting_for_data_)
        done();

    hud_.time_tick(time_delta);
    elapsed_ = time_delta;

    if (!waiting_for_data_)
    {
        auto lock (entities_.acquire_write_lock());
        double delta (time_delta);
        system_lag_compensate(entities_, time_delta, player_entity_);

        while (delta > 0)
        {
            constexpr double max_step = 0.05;
            double step;
            if (delta > max_step)
            {
                step = max_step;
                delta -= max_step;
            }
            else if (delta < 0.001)
            {
                break;
            }
            else
            {
                step = delta;
                delta = 0;
            }

            system_gravity(entities_, step);
            system_walk(entities_, step);
            system_motion(entities_, step);
            system_terrain_collision(entities_,
                [&](chunk_coordinates c) -> boost::optional<const surface_data&>
                {
                    if (!map_->is_surface_available(c))
                        return boost::optional<const surface_data&>();

                    return map_->get_surface(c);
                },
                [&](chunk_coordinates c)
                {
                    return is_air_chunk(c, map_->get_coarse_height(c));
                }
            );

            system_terrain_friction(entities_, step);
        }
    }

    on_tick(time_delta);
    player_controls();
    player_motion();
}

void main_game::render()
{
    if (player_entity_ != 0xffffffff)
    {
        try
        {
            auto lock (entities_.acquire_read_lock());
            wfpos pp (entities_.get<wfpos>(player_entity_, entity_system::c_position));
            pp.normalize();
            player_.move_to(pp);
            player_.velocity = entities_.get<vector>(player_entity_, entity_system::c_velocity);
            player_.is_airborne = (entities_.get<vector>(player_entity_, entity_system::c_impact).z == 0);
            hud_.local_height = map().get_coarse_height(pp.int_pos() / chunk_size);
        }
        catch(...)
        {
        }
    }

    scene_.pre_render();

    {
    std::unique_lock<std::mutex> scene_lock (scene_.lock);

    renderer().prepare(player_);
    renderer().opaque_pass();
    {
    auto lock (entities_.acquire_read_lock());

    entities_.for_each<wfpos>(entity_system::c_position,
        [&](es::storage::iterator i,
            wfpos& p) -> uint64_t
    {
        p.normalize();
        renderer().draw_model(p, 0);

        if (entities_.entity_has_component(i, entity_system::c_name))
        {

        }

        return 0;
    });
    } // entities_ lock

    renderer().handle_occlusion_queries();
    renderer().transparent_pass();

    } // scene_.lock

    if (show_ui_)
    {
    // The block highlight color blinks smoothly.
    float alpha (std::sin(game_.total_time_passed() * 4.) * 0.02f + 0.12f);
    color_alpha hl_color (1,1,1, alpha);

    world_coordinates offset (renderer().offset() * chunk_size);
    vector origin (player_.rel_world_position(offset));
    origin.z += 1.7f; // Dirty hack to get to the eye level, TODO

    auto line (voxel_raycast(origin, origin + from_spherical(player_.head_angle()) * 20.f));
    for (auto i (std::next(line.begin())); i != line.end(); ++i)
    {
        world_coordinates block_pos (*i + offset);
        auto cpos (block_pos / chunk_size);

        if (!map().is_surface_available(cpos))
            continue;

        auto& surf (map().get_surface(cpos));

        // Look for the current block in both the transparent and opaque
        // surfaces.
        auto found (boost::range::find(surf.opaque, block_pos % chunk_size));
        if (found == surf.opaque.end())
        {
            found = boost::range::find(surf.transparent, block_pos % chunk_size);
            if (found == surf.transparent.end())
                continue;
        }

        auto coll_block (found->type);
        assert(coll_block != 0); // No need to check for air blocks.

        // If it's a normal block, we found an intersection.
        auto& coll_material (material_prop[coll_block]);
        if (!coll_material.is_custom_block())
        {
            renderer().highlight_face({offset + *i, offset + *(i-1)},
                                      hl_color);
            break;
        }

        // It's a custom model; we'll need to do a detailed raycast
        // against every component.
        ray<float> pr ((origin - vector(*i)) * 16.f, player_.head_angle());
        bool intersected (false);
        for (auto& part : coll_material.model)
        {
            intersected = ray_box_intersection(pr, part.bounding_box());
            if (intersected)
            {
                renderer().highlight_custom_block(offset + *i,
                                                  coll_material.model,
                                                  hl_color);
                break;
            }
        }

        if (intersected)
            break;
    }
    }

    if (show_ui_)
    {
        renderer().draw_ui(elapsed_, hud_);
        player_.hotbar_needs_update = false;
        hud_.hotbar_needs_update = false;
    }

    scene_.post_render();
}

bool main_game::process_event(const event& ev)
{
    if (game_.mouse_is_relative())
        process_event_captured(ev);
    else
        process_event_uncaptured(ev);

    return true;
}

bool main_game::process_event (const sf::Event& ev)
{
    if (!game_.mouse_is_relative())
        renderer().process(ev);

    return true;
}

void main_game::process_event_captured (const event& ev)
{
    auto& p (player_);
    auto old_active_slot (hud_.active_slot);

    switch (ev.type)
    {
    case event::window_close:
        stop();
        break;

    case event::key_up:
        switch (ev.keycode)
        {
            case key::space:
                action_stop(0); break;

            case key::esc:
                game_.relative_mouse(false);

            default: ; // do nothing
        }
        break;

    case event::key_down:
        switch (ev.keycode)
        {
        case key::esc:
            if (hud_.show_input())
            {
                hud_.set_input(std::u32string());
                hud_.show_input(false);
            }
            else
            {
                stop();
            }
            break;

        case key::num0: hud_.active_slot = 1; break;
        case key::num1: hud_.active_slot = 2; break;
        case key::num2: hud_.active_slot = 3; break;
        case key::num3: hud_.active_slot = 4; break;
        case key::num4: hud_.active_slot = 5; break;
        case key::num5: hud_.active_slot = 6; break;
        case key::num6: hud_.active_slot = 7; break;
        case key::num7: hud_.active_slot = 8; break;
        case key::num8: hud_.active_slot = 9; break;
        case key::num9: hud_.active_slot =10; break;

        //case key::space:
        //    action(0); break;

        case key::l_bracket:
            if (!hud_.show_input())
            {
                auto vd (scene_.view_distance());
                if (vd > 4)
                {
                    --vd;
                    scene_.view_distance(vd);
                    hud_.console_message((format("View distance decreased to %1%.") % (vd * chunk_size)).str());
                }
            }
            break;

        case key::r_bracket:
            if (!hud_.show_input())
            {
                auto vd (scene_.view_distance());
                if (vd < 64)
                {
                    ++vd;
                    scene_.view_distance(vd);
                    hud_.console_message((format("View distance increased to %1%.") % (vd * chunk_size)).str());
                }
            }
            break;

        case key::f1:
            show_ui_ = !show_ui_;
            break;

        case key::f2:
            hud_.console_message("Screenshot saved.");
            // The screenshot save routine itself is in game.cpp, here we
            // just augment it with a console message.
            break;

        case key::f3:
            hud_.show_debug_info = !hud_.show_debug_info;
            break;

        case key::t:
        case key::slash:
            if (!hud_.show_input())
            {
                hud_.show_input(true);
                hud_.set_cursor(0);
                hud_.set_input(std::u32string());
                if (ev.keycode == key::t)
                    ignore_text_ = 't';
            }
            break;

        case key::backspace:
            if (hud_.show_input())
            {
                auto str (hud_.get_input());
                auto pos (hud_.get_cursor());
                if (pos > 0)
                {
                    --pos;
                    str.erase(str.begin() + pos);
                    hud_.set_input(str);
                    hud_.set_cursor(pos);
                }
            }
            break;


        case key::enter:
            if (hud_.show_input())
            {
                console_input(hud_.get_input());
                hud_.show_input(false);
            }
            break;

        default: ; // do nothing

        }
        break;

    case event::key_text:
        if (ignore_text_ != ev.code && hud_.show_input() && std::isprint(ev.code))
        {
            std::u32string str (hud_.get_input());
            str.insert(str.begin() + hud_.get_cursor(), ev.code);
            hud_.set_input(str);
            hud_.set_cursor(hud_.get_cursor() + 1);
        }
        ignore_text_ = 0;
        break;

    case event::mouse_move_rel:
        {
        const float rate_of_turn (0.001f); // radians per pixel
        p.turn_head(vector2<float>(ev.xy) * rate_of_turn);
        }
        break;

    case event::mouse_button_down:
        action(ev.code + 1);
        break;

    case event::mouse_button_up:
        action_stop(ev.code + 1);
        break;

    case event::mouse_wheel:
        if (ev.delta > 0)
        {
            hud_.active_slot++;
            if (hud_.active_slot >= hud_.bar.size())
                hud_.active_slot = 0;
        }
        else
        {
            if (hud_.active_slot == 0)
                hud_.active_slot = hud_.bar.size() - 1;
            else
                hud_.active_slot--;
        }
        break;

    case event::joy_button_up:
        action_stop(ev.code);
        break;

    case event::joy_button_down:
        if (ev.code == 8)
            stop();
        else
            action(ev.code);

        break;

    default:
        ;
    }

    if (hud_.active_slot != old_active_slot)
        hud_.hotbar_needs_update = true;
}

void main_game::process_event_uncaptured (const event& ev)
{
    switch (ev.type)
    {
        case event::window_close:
            stop();
            break;

        case event::key_up:
            switch (ev.keycode)
            {
                case key::esc:
                    game_.relative_mouse(true);

                default: ; // do nothing
            }
            break;

        default: ; // do nothing
    }
}

void main_game::stop()
{
    stop_ = true;
    clock_.join();
    done();
}

double main_game::elapsed_time()
{
    return elapsed_;
}

game_state::transition main_game::next_state() const
{
    if (!loading_screen_)
    {
        loading_screen_ = true;
        return game_state::transition(game_.make_state<loading_screen>(waiting_for_data_), false);
    }
    assert(stop_);
    return game_state::transition();
}

void main_game::player_motion()
{
    if (old_chunk_pos_ != player_.chunk_position())
        scene_.move_camera_to(player_.chunk_position());

    old_chunk_pos_ = player_.position() / chunk_size;
}

void main_game::console_input(const std::u32string &msg)
{
    msg::console m;
    m.text = utf32_to_utf8(msg);
    send(serialize_packet(m), m.method());
}

void main_game::login()
{
    clock_ = boost::thread([&]{ bg_thread(); });

    msg::login m;

    m.protocol_version = 1;
    std::string method;
    if (singleplayer_)
        method = "singleplayer";
    else
        method = "ecdh";

    auto plr_info (get_player_info());
    std::stringstream json;
    pt::ptree info;
    info.put("name", plr_info.name);
    info.put("uid", plr_info.uid);
    info.put("public_key", plr_info.public_key);
    info.put("method", method);
    pt::json_parser::write_json(json, info);

    m.credentials = json.str();

    send(serialize_packet(m), m.method());
}

void main_game::request_chunk(const chunk_coordinates& pos)
{
    boost::mutex::scoped_lock lock (requests_lock_);
    requests_.insert(pos);
}

player& main_game::get_player()
{
    return player_;
}

std::unique_ptr<terrain_mesher_i>
main_game::make_terrain_mesher()
{
    return renderer().make_terrain_mesher();
}

void main_game::walk(float dir, float speed)
{
    if (player_entity_ == 0xffffffff)
        return;

    vector2<float> move (std::sin(dir), std::cos(dir));

    const float walk_force (1.0f);
    float magnitude (walk_force * speed);
    auto lock (entities_.acquire_write_lock());
    entities_.set(player_entity_, entity_system::c_walk, move * magnitude);
}

void main_game::action(uint8_t code)
{
    msg::button_press msg (code, hud_.active_slot, player_.head_angle(), player_.get_wfpos());
    send(serialize_packet(msg), msg.method());

    if (code == 0 && player_entity_ != 0xffffffff)
    {
        auto lock (entities_.acquire_write_lock());
        if (entities_.get<vector>(player_entity_, entity_system::c_impact).z > 0)
        {
            auto v (entities_.get<vector>(player_entity_, entity_system::c_velocity));
            v.z = 6.0f;
            entities_.set(player_entity_, entity_system::c_velocity, v);
        }
    }
}

void main_game::action_stop(uint8_t code)
{
    msg::button_release msg (code);
    send(serialize_packet(msg), msg.method());
}

void main_game::receive (const packet& p)
{
    auto archive (make_deserializer(p));

    unsigned int mt (p.message_type());

    try
    {
        switch(p.message_type())
        {
        case msg::handshake::msg_id:
            handshake(archive); break;
        case msg::greeting::msg_id:
            greeting(archive); break;
        case msg::kick::msg_id:
            kick(archive); break;
        case msg::time_sync_response::msg_id:
            time_sync_response(archive); break;
        case msg::define_resources::msg_id:
            define_resources(archive); break;
        case msg::define_materials::msg_id:
            define_materials(archive); break;
        case msg::entity_update::msg_id:
            entity_update(archive); break;
        case msg::entity_update_physics::msg_id:
            entity_update_physics(archive); break;
        case msg::entity_delete::msg_id:
            entity_delete(archive); break;
        case msg::surface_update::msg_id:
            surface_update(archive); break;
        case msg::lightmap_update::msg_id:
            lightmap_update(archive); break;
        case msg::heightmap_update::msg_id:
            heightmap_update(archive); break;
        case msg::player_configure_hotbar::msg_id:
            configure_hotbar(archive); break;
        case msg::global_config::msg_id:
            global_config(archive); break;
        case msg::print_msg::msg_id:
            print_msg(archive); break;

        default:
            log_msg("Unknown packet type %1%", mt);
        }
    }
    catch (std::exception& e)
    {
        log_msg("Cannot parse packet type %1%: %2%", mt, e.what());
    }
}

void main_game::handshake (deserializer<packet>& p)
{
    msg::handshake mesg;
    mesg.serialize(p);
    log_msg("Connected to %1%", mesg.server_name );
}

void main_game::greeting (deserializer<packet>& p)
{
    msg::greeting mesg;
    mesg.serialize(p);

    clock::sync(mesg.client_time);
    log_msg("initial clock synced to %1%", mesg.client_time);

    player_entity_ = mesg.entity_id;

    renderer().offset(mesg.position >> cnkshift);
    player_.move_to(mesg.position, vector3<float>(.5f, .5f, 5.f));

    log_msg("MOTD: %1%", mesg.motd);
    log_msg("player %1% spawned at %2%", mesg.entity_id, mesg.position);

    msg::time_sync_request sync;
    sync.request = clock::time();
    send(serialize_packet(sync), sync.method());
}

void main_game::kick (deserializer<packet>& p)
{
    msg::kick mesg;
    mesg.serialize(p);

    trace("Got kicked: %1%", mesg.reason);
    log_msg("Got kicked: %1%", mesg.reason);
    disconnect();
}

void main_game::time_sync_response (deserializer<packet>& p)
{
    msg::time_sync_response mesg;
    mesg.serialize(p);
    auto roundtrip (clock::time() - mesg.request);
    clock::sync(mesg.response + roundtrip * 0.5);
}

void main_game::define_resources (deserializer<packet>& p)
{
    msg::define_resources msg;
    msg.serialize(p);

    renderer().load_textures(msg.textures);

    log_msg("Registered %1% textures and %2% models", msg.textures.size(), msg.models.size());
}

void main_game::define_materials (deserializer<packet>& p)
{
    msg::define_materials msg;
    msg.serialize(p);

    for (auto& rec : msg.materials)
        register_new_material(rec.material_id) = rec.definition;

    log_msg("Registered %1% materials", msg.materials.size());
}

void main_game::entity_update (deserializer<packet>& p)
{
    msg::entity_update msg;
    msg.serialize(p);

    auto lock (entities_.acquire_write_lock());

    for (auto& upd : msg.updates)
    {
        auto e (entities_.make(upd.entity_id));
        switch (upd.component_id)
        {
        case entity_system::c_position:
            entities_.set_position(e, deserialize_as<wfpos>(upd.data));
            break;

        case entity_system::c_velocity:
        case entity_system::c_force:
        case entity_system::c_boundingbox:
            entities_.set(e, upd.component_id,
                          deserialize_as<vector>(upd.data));
            break;

        case entity_system::c_orientation:
            entities_.set_orientation(e, deserialize_as<float>(upd.data));
            break;

        case entity_system::c_model:
            entities_.set_model(e, deserialize_as<uint16_t>(upd.data));
            break;

        case entity_system::c_name:
            entities_.set_name(e, deserialize_as<std::string>(upd.data));
            break;

        case entity_system::c_hotbar:
            entities_.set_hotbar(e, deserialize_as<hotbar>(upd.data));

            hud_.bar = deserialize_as<hotbar>(upd.data);
            hud_.hotbar_needs_update = true;

            break;
        }
    }
}

void main_game::entity_update_physics (deserializer<packet>& p)
{
    //////////////////////////////////////
    //static bool first (true);
    //if (!first) return;
    //first = false;

    msg::entity_update_physics msg;
    msg.serialize(p);

    //trace("Update entity physics");
    int32_t lag_msec (clock::time() - msg.timestamp);
    float   lag (lag_msec * 0.001f);

    auto lock (entities_.acquire_write_lock());

    for (auto& upd : msg.updates)
    {
        auto e (entities_.make(upd.entity_id));
        auto newpos (upd.pos + upd.velocity * lag);

        //trace("Set entity %1% to position %2%", upd.entity_id, upd.pos);
        //trace("  velocity %1%, lag %2%", upd.velocity, lag);

        if (   entities_.entity_has_component(e, entity_system::c_position)
            && entities_.entity_has_component(e, entity_system::c_velocity))
        {
            last_known_phys info { newpos, upd.velocity };
            entities_.set(e, entity_system::c_lag_comp, info);
        }
        else
        {
            entities_.set_position(e, newpos);
            entities_.set_velocity(e, upd.velocity);
        }
    }
}

void main_game::entity_delete (deserializer<packet>& p)
{
    msg::entity_delete msg;
    msg.serialize(p);

    log_msg("Delete entity %1%", msg.entity_id);
    auto lock (entities_.acquire_write_lock());
    entities_.delete_entity(msg.entity_id);
}

void main_game::surface_update (deserializer<packet>& p)
{
    waiting_for_data_ = false;

    msg::surface_update msg;
    msg.serialize(p);

    trace("receive surface %1%", msg.position);

    map().store_surface(msg.position, msg.terrain);
    if (msg.light.unpacked_len > 0)
    {
        map().store_lightmap(msg.position, msg.light);
        scene_.set(msg.position,
                   map().get_surface(msg.position),
                   map().get_lightmap(msg.position));
    }
    else
    {
        assert(false);
    }
}

void main_game::lightmap_update (deserializer<packet>& p)
{
    msg::lightmap_update msg;
    msg.serialize(p);

    trace("Lightmap update for %1%", msg.position);

    map().store_lightmap(msg.position, msg.data);

    scene_.set(msg.position,
               map().get_surface(msg.position),
               map().get_lightmap(msg.position));
}

void main_game::heightmap_update (deserializer<packet>& p)
{
    msg::heightmap_update msg;
    msg.serialize(p);

    for (auto& r : msg.data)
    {
        auto old_height (map().get_coarse_height(r.pos));
        if (r.height != old_height)
        {
            map().store_coarse_height(r.pos, r.height);
            scene_.set_coarse_height(r.pos, r.height, old_height);
        }
    }
}

void main_game::configure_hotbar (deserializer<packet>& p)
{
    msg::player_configure_hotbar msg;
    msg.serialize(p);

    //hud_.hotbar = msg.slots;
    //hud_.hotbar_needs_update = true;
}

void main_game::global_config (deserializer<packet>& p)
{
    msg::global_config msg;
    msg.serialize(p);
}

void main_game::print_msg (deserializer<packet>& p)
{
    msg::print_msg msg;
    msg.serialize(p);

    hud_.console_message(msg.json);
}

void main_game::bg_thread()
{
    static size_t count (0);
    while(!stop_)
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
        poll(0);
        ++count;

        // Every 2 seconds, see if we can get flush some chunks from memory.
        if (count % 2000 == 0)
        {
            map().cleanup();
        }

        boost::mutex::scoped_lock lock (requests_lock_);
        if (!requests_.empty())
        {
            std::unordered_set<map_coordinates> missing_height;

            msg::request_surfaces req;
            //for (auto& pos : requests_)
            size_t count2 (0);
            for (auto i (requests_.begin()); i != requests_.end(); )
            {
                if (++count2 > 2000)
                {
                    log_msg("Warning: request queue full");
                    break;
                }

                auto& pos (*i);
                if (!map().is_coarse_height_available(pos))
                {
                    missing_height.insert(pos);
                    req.requests.emplace_back(pos, 0);
                }
                else if (is_air_chunk(pos, map().get_coarse_height(pos)))
                {
                    trace("Tried to send request for air chunk");
                }
                else if (map().is_surface_available(pos))
                {
                    trace("Request for surface I already have, %1%", map().get_surface(pos).version);
                    req.requests.emplace_back(pos, map().get_surface(pos).version);
                }
                else
                {
                    req.requests.emplace_back(pos, 0);
                }
                i = requests_.erase(i);
            }

            if (!missing_height.empty())
            {
                msg::request_heights rqh;
                copy(missing_height, std::back_inserter(rqh.requests));
                send(serialize_packet(rqh), rqh.method());
            }

            send(serialize_packet(req), req.method());
        }
    }
}

} // namespace hexa

