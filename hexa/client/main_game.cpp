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
// Copyright 2012-2013, nocte@hippie.nu
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

#include <SFML/Graphics/Image.hpp>

#include <es/component.hpp>
#include <es/entity.hpp>

#include <hexa/algorithm.hpp>
#include <hexa/collision.hpp>
#include <hexa/compression.hpp>
#include <hexa/chunk.hpp>
#include <hexa/config.hpp>
#include <hexa/os.hpp>
#include <hexa/protocol.hpp>
#include <hexa/lightmap.hpp>
#include <hexa/ray.hpp>
#include <hexa/voxel_algorithm.hpp>
#include <hexa/physics/euler_integrater.hpp>
#include <hexa/process.hpp>
#include <hexa/trace.hpp>
#include <hexa/voxel_range.hpp>

#include "clock.hpp"
#include "event.hpp"
#include "renderer_i.hpp"
#include "sfml_ogl2.hpp"
#include "sfml_ogl3.hpp"
#include "../persistence_null.hpp"
#include "../persistence_sqlite.hpp"
#include "../memory_cache.hpp"
#include "../entity_system.hpp"
#include "../entity_system_physics.hpp"

#include "loading_screen.hpp"

using namespace boost;
using namespace boost::math::float_constants;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace hexa {

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
    , singleplayer_  (host == "localhost")
{
    if (singleplayer_)
    {
        try
        {
            fs::path dir (executable_path().parent_path());
            server_process_ = start_process(dir / "hexahedra-server", { });
        }
        catch (std::runtime_error& e)
        {
            std::cout << "Cannot launch server: " << e.what() << std::endl
                      << "Still attempting to connect..." << std::endl;
        }
    }

    hud_.console_message(u8"Test. \u00A9\u00C6\u0270\u03C1");
    hud_.time_tick(1);
    hud_.console_message(u8"Эксперты: здоровью Филина причинен тяжкий вред");
    hud_.time_tick(1);
    hud_.console_message(u8"España");
    hud_.time_tick(1);
    hud_.console_message(u8"Ὕμνος εἰς τὴν Ἐλευθερίαν");

    game_.relative_mouse(true);
    setup_renderer();
    setup_world(host, port);
    scene_.view_distance(vd);
    renderer().view_distance(vd * 3);

    renderer().on_new_vbo.connect([&](chunk_coordinates pos)
        { scene_.send_visibility_requests(pos); });

    std::cout << "Trying to connect to " << host << ":" << port << std::endl;
    int tries (0);
    while (!connect())
    {
        std::cout << "   ... retrying..." << std::endl;
        ++tries;
        if (tries > 2)
        {
            if (singleplayer_)
                terminate_process(server_process_);

            throw std::runtime_error("cannot connect to server");
        }
    }
    std::cout << "Connected!" << std::endl;

    login();
    std::cout << "Logged in!" << std::endl;
}

main_game::~main_game()
{
    clock_.join();

    if (singleplayer_)
        terminate_process(server_process_);
}

void main_game::setup_world (const std::string& host, uint16_t port)
{
    extern po::variables_map global_settings;

    fs::path user_dir (global_settings["userdir"].as<std::string>());
    fs::path data_dir (global_settings["datadir"].as<std::string>());

    fs::path gameroot (user_dir / "games");
    std::string host_id ((format("%1%:%2%") % host % port).str());
    fs::path gamepath (gameroot / host_id);

    fs::path db (gamepath / "world.db");

    if (   !singleplayer_
        && !fs::exists(gamepath)
        && !fs::create_directories(gamepath))
    {
        throw std::runtime_error((format("error: cannot create directory '%1%'") % gamepath.string()).str());
    }

    if (singleplayer_)
        //aux_.reset(new persistence_sqlite(io_, "local.db", data_dir / "dbsetup.sql"));
        aux_ = make_unique<persistence_null>();
    else
        aux_ = make_unique<persistence_sqlite>(io_, db, data_dir / "dbsetup.sql");

    world_ = make_unique<memory_cache>(*aux_);
}

void main_game::setup_renderer()
{
    // Pick a renderer based on the OpenGL driver and the program settings.

    extern po::variables_map global_settings;

    std::string ogl_version ((const char*)glGetString(GL_VERSION));
    std::cout << "Driver: " << ogl_version << std::endl;
    int ogl_major (ogl_version[0] - '0'), ogl_minor (ogl_version[2] - '0');
    if (ogl_major == 1 && ogl_minor < 5)
    {
        throw std::runtime_error("Sorry, you need at least OpenGL 1.5");
    }

    if (   (ogl_major == 1 && ogl_minor >= 5) || ogl_major == 2
        || global_settings.count("ogl2"))
    {
        // OpenGL 1.5 or 2.x
        renderer_ = make_unique<sfml_ogl2>(window());
    }
    else
    {
        // OpenGL 3 or newer
        renderer_ = make_unique<sfml_ogl3>(window());
    }
}


void main_game::set_view_distance(unsigned int d)
{
    scene_.view_distance(d);
    renderer().view_distance(d * 3);
}

void main_game::resize (unsigned int w, unsigned int h)
{
    renderer().resize(w, h);
}

void main_game::player_controls()
{
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
            entities_.set(player_entity_, entity_system::c_lookat, look.look);
    }
}

void main_game::update(double time_delta)
{
    if (waiting_for_data_)
        done();

    hud_.time_tick(time_delta);
    elapsed_ = time_delta;

    {
    //boost::mutex::scoped_lock lock (entities_mutex_);
    system_gravity(entities_, time_delta);
    system_walk(entities_, time_delta);
    system_motion(entities_, time_delta);
    system_terrain_collision(entities_, world());
    system_terrain_friction(entities_, time_delta);
    //system_lag_compensate(entities_, time_delta);
    } // scoped  lock

    on_tick(time_delta);
    player_controls();
    player_motion();

    {
    mutex::scoped_lock lock2 (scene_.lock);
    renderer().remove_chunks(scene_.to_be_deleted);
    scene_.to_be_deleted.clear();

    for (chunk_coordinates pos : scene_.new_occlusion_queries)
        renderer().add_occlusion_query(pos);

    scene_.new_occlusion_queries.clear();

    for (chunk_coordinates pos : renderer().get_visible_queries())
        scene_.make_chunk_visible(pos);
    } // scoped  lock
}

void main_game::render()
{
    boost::mutex::scoped_lock lock (entities_mutex_);

    if (player_entity_ != 0xffffffff)
    {
        try
        {
            wfpos pp (entities_.get<wfpos>(player_entity_, entity_system::c_position));
            pp.normalize();
            player_.move_to(pp);
            player_.velocity = entities_.get<vector>(player_entity_, entity_system::c_velocity);
            player_.is_airborne = (entities_.get<vector>(player_entity_, entity_system::c_impact).z == 0);
        }
        catch(...)
        {
        }
    }

    scene_.on_pre_render_loop();
    renderer().prepare(player_);
    renderer().opaque_pass();

    {

    wfpos plp (player_.position(), player_.position_fraction());
    renderer().draw_model(plp, 0);

    entities_.for_each<wfpos>(entity_system::c_position,
        [&](es::storage::iterator i,
            es::storage::var_ref<wfpos> pos)
    {
        renderer().draw_model(pos, 0);
    });
    }

    renderer().handle_occlusion_queries();
    renderer().transparent_pass();
    renderer().draw_ui(elapsed_, hud_);

    player_.hotbar_needs_update = false;
    hud_.hotbar_needs_update = false;
}

void main_game::process_event (const event& ev)
{
    if (game_.mouse_is_relative())
        process_event_captured(ev);
    else
        process_event_uncaptured(ev);
}

void main_game::process_event (const sf::Event& ev)
{
    if (!game_.mouse_is_relative())
        renderer().process(ev);
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

            default: /* do nothing */ ;
        }
        break;

    case event::key_down:
        switch (ev.keycode)
        {
        case key::esc:
            stop(); break;

        case key::num0: hud_.active_slot = 0; break;
        case key::num1: hud_.active_slot = 1; break;
        case key::num2: hud_.active_slot = 2; break;
        case key::num3: hud_.active_slot = 3; break;
        case key::num4: hud_.active_slot = 4; break;
        case key::num5: hud_.active_slot = 5; break;
        case key::num6: hud_.active_slot = 6; break;
        case key::num7: hud_.active_slot = 7; break;
        case key::num8: hud_.active_slot = 8; break;
        case key::num9: hud_.active_slot = 9; break;

        case key::space:
            action(0); break;

        case key::l_bracket:
            {
            auto vd (scene_.view_distance());
            if (vd > 4)
            {
                --vd;
                scene_.view_distance(vd);
                renderer().view_distance(vd * 3);
                hud_.console_message((format("View distance decreased to %1%.") % (vd * chunk_size)).str());
            }
            }
            break;

        case key::r_bracket:
            {
            auto vd (scene_.view_distance());
            if (vd < 64)
            {
                ++vd;
                scene_.view_distance(vd);
                renderer().view_distance(vd * 3);
                hud_.console_message((format("View distance increased to %1%.") % (vd * chunk_size)).str());
            }
            }
            break;

        default: /* do nothing */ ;

        }
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
            if (hud_.active_slot >= hud_.hotbar.size())
                hud_.active_slot = 0;
        }
        else
        {
            if (hud_.active_slot == 0)
                hud_.active_slot = hud_.hotbar.size() - 1;
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

                default: /* do nothing */ ;
            }
            break;

        default:
            ;
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
    if (waiting_for_data_)
        return { game_.make_state<loading_screen>(waiting_for_data_), false };

    return { nullptr, false };
}

void main_game::player_motion()
{
    if (old_chunk_pos_ != player_.chunk_position())
    {
        boost::mutex::scoped_lock lock (scene_.lock);
        scene_.on_move(player_.chunk_position());
    }
    old_chunk_pos_ = player_.position() / chunk_size;
}

void main_game::login()
{
    clock_ = boost::thread([&]{ bg_thread(); });

    msg::login m;

    m.protocol_version = 1;
    m.username = "Griefy McGriefenstein";

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

    const float walk_force (40.0f);
    float magnitude (walk_force * speed);
    entities_.set(player_entity_, entity_system::c_walk, move * magnitude);
}

void main_game::action(uint8_t code)
{
    msg::button_press msg (code, player_.active_slot, player_.head_angle());
    send(serialize_packet(msg), msg.method());

    if (code == 0 && player_entity_ != 0xffffffff)
    {
        if (entities_.get<vector>(player_entity_, entity_system::c_impact).z > 0)
        {
            auto v (entities_.get<vector>(player_entity_, entity_system::c_velocity));
            v.z += 7.0f;
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

    try
    {
        switch(p.message_type())
        {
        case msg::handshake::msg_id:
            handshake(archive); break;
        case msg::greeting::msg_id:
            greeting(archive); break;
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

        default:
            std::cout << "Unknown packet type " << p.message_type() << std::endl;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Cannot parse packet : " << e.what() << std::endl;
    }
}

void main_game::handshake (deserializer<packet>& p)
{
    msg::handshake mesg;
    mesg.serialize(p);
    std::cout << "Connected to " << mesg.server_name << std::endl;
}

void main_game::greeting (deserializer<packet>& p)
{
    msg::greeting mesg;
    mesg.serialize(p);

    clock::sync(mesg.client_time);
    trace("initial clock synced to %1%", mesg.client_time);

    player_entity_ = mesg.entity_id;

    renderer().set_offset(mesg.position);
    player_.move_to(mesg.position, vector3<float>(.5f, .5f, 5.f));

    trace(mesg.motd);
    trace("player %1% spawned at %2%", mesg.entity_id, mesg.position);

    msg::time_sync_request sync;
    sync.request = clock::time();
    send(serialize_packet(sync), sync.method());
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

    std::cout << "Registered " << msg.textures.size() << " textures."
              << std::endl;

    renderer().load_textures(msg.textures);

    std::cout << "Registered " << msg.models.size() << " models."
              << std::endl;
}

void main_game::define_materials (deserializer<packet>& p)
{
    msg::define_materials msg;
    msg.serialize(p);

    for (auto& rec : msg.materials)
        register_new_material(rec.material_id) = rec.definition;

    std::cout << "Registered " << msg.materials.size() << " materials."
              << std::endl;
}

void main_game::entity_update (deserializer<packet>& p)
{
    msg::entity_update msg;
    msg.serialize(p);

    boost::mutex::scoped_lock lock (entities_mutex_);

    for (auto& upd : msg.updates)
    {
        auto e (entities_.make(upd.entity_id));
        switch (upd.component_id)
        {
        case entity_system::c_position:
            entities_.set(e, upd.component_id,
                          deserialize_as<wfpos>(upd.data));
            break;

        case entity_system::c_velocity:
        case entity_system::c_force:
        case entity_system::c_boundingbox:
            entities_.set(e, upd.component_id,
                          deserialize_as<vector>(upd.data));
            break;

        case entity_system::c_orientation:
            entities_.set(e, upd.component_id,
                          deserialize_as<float>(upd.data));
            break;

        case entity_system::c_model:
            entities_.set(e, upd.component_id,
                          deserialize_as<uint16_t>(upd.data));
            break;

        case entity_system::c_name:
            entities_.set(e, upd.component_id,
                          deserialize_as<std::string>(upd.data));
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

    int32_t lag_msec (clock::time() - msg.timestamp);
    float   lag (lag_msec * 0.001f);

    boost::mutex::scoped_lock lock (entities_mutex_);

    for (auto& upd : msg.updates)
    {
        auto e (entities_.make(upd.entity_id));
        auto newpos (upd.pos + upd.velocity * lag);

        if (   entities_.entity_has_component(e, entity_system::c_position)
            && entities_.entity_has_component(e, entity_system::c_velocity))
        {
            last_known_phys info { newpos, upd.velocity };
            entities_.set(e, entity_system::c_lag_comp, info);
        }
        else
        {
            entities_.set(e, entity_system::c_position, newpos);
            entities_.set(e, entity_system::c_velocity, upd.velocity);
        }
    }
}

void main_game::surface_update (deserializer<packet>& p)
{
    waiting_for_data_ = false;

    msg::surface_update msg;
    msg.serialize(p);

    auto temp (decompress(msg.terrain));
    surface_ptr s (new surface_data(deserialize_as<surface_data>(temp)));

    world().store(msg.position, s);
    lightmap_ptr lm (world().get_lightmap(msg.position));
    if (lm == nullptr)
        lm.reset(new light_data);

    auto tmpl (decompress(msg.light));
    (*lm) = deserialize_as<light_data>(tmpl);
    world().store(msg.position, lm);

    assert(count_faces(s->opaque) == lm->opaque.size());
    assert(count_faces(s->transparent) == lm->transparent.size());

    boost::mutex::scoped_lock lock (scene_.lock);
    scene_.on_update_chunk(msg.position);
}

void main_game::lightmap_update (deserializer<packet>& p)
{
    msg::lightmap_update msg;
    msg.serialize(p);

    if (!world().is_surface_available(msg.position))
    {
        std::cout << "Error: light map update without surface " << msg.position << std::endl;
        return;
    }

    if (msg.data.unpacked_len == 0)
        return;

    lightmap_ptr lm (world().get_lightmap(msg.position));
    if (lm == nullptr)
        lm.reset(new light_data);

    auto tmpl (decompress(msg.data));
    (*lm) = deserialize_as<light_data>(tmpl);
    world().store(msg.position, lm);

    {
    boost::mutex::scoped_lock lock (scene_.lock);
    scene_.on_update_chunk(msg.position);
    }
}

void main_game::heightmap_update (deserializer<packet>& p)
{
    msg::heightmap_update msg;
    msg.serialize(p);

    boost::mutex::scoped_lock lock (scene_.lock);
    for (auto& r : msg.data)
    {
        //trace("height at %1%: %2%", r.pos, r.height);

        world().store(r.pos, r.height);
        scene_.on_update_height(r.pos, r.height);
        renderer().on_update_height(r.pos, r.height);
    }
}

void main_game::configure_hotbar (deserializer<packet>& p)
{
    msg::player_configure_hotbar msg;
    msg.serialize(p);

    std::cout << "Hotbar msg " << msg.slots.size() << std::endl;

    hud_.hotbar = msg.slots;
    hud_.hotbar_needs_update = true;
}


void main_game::global_config (deserializer<packet>& p)
{
    msg::global_config msg;
    msg.serialize(p);
/*
    if (msg.name == "game icon")
    {
        trace("game icon, png size %1%", msg.value.size());
        try
        {
            sf::Image icon;
            if (icon.loadFromMemory(&msg.value[0], msg.value.size()))
            {
                window().setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
                trace("icon %1%x%2% set succesfully", icon.getSize().x, icon.getSize().y);
            }
        }
        catch (...)
        {
            trace("failed to load game icon");
        }
    }
    */
}

void main_game::bg_thread()
{
    static size_t count (0);
    while(!stop_)
    {
        //boost::this_thread::sleep(boost::posix_time::milliseconds(100));
        poll(100);
        ++count;

        // Every 2 seconds, see if we can get flush some chunks from memory.
        if (count % 20 == 0)
        {
            world().cleanup();
        }

        boost::mutex::scoped_lock lock (requests_lock_);
        if (!requests_.empty())
        {
            std::unordered_set<map_coordinates> missing_height;

            msg::request_chunks req;
            //for (auto& pos : requests_)
            size_t count (0);
            for (auto i (requests_.begin()); i != requests_.end(); )
            {
                if (++count > 20000)
                {
                    std::cout << "Warning: request queue full" << std::endl;
                    break;
                }

                auto& pos (*i);
                if (!world().is_coarse_height_available(pos))
                {
                    missing_height.insert(pos);
                    req.requests.emplace_back(pos, 0);
                }
                else if (!is_air_chunk(pos, world().get_coarse_height(pos)))
                {
                    req.requests.emplace_back(pos, 0);
                }
                else
                {
                    trace("Tried to send request for air chunk");
                    scene_.on_update_chunk(pos);
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

