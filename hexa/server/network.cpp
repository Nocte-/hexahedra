//---------------------------------------------------------------------------
// server/network.cpp
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

#include "network.hpp"

#include <chrono>
#include <iostream>
#include <map>
#include <thread>

#include <boost/format.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/program_options.hpp>

#include <hexa/compression.hpp>
#include <hexa/entity_system.hpp>
#include <hexa/entity_system_physics.hpp>
#include <hexa/geometric.hpp>
#include <hexa/log.hpp>
#include <hexa/protocol.hpp>
#include <hexa/trace.hpp>
#include <hexa/voxel_algorithm.hpp>
#include <hexa/voxel_range.hpp>

#include "clock.hpp"
#include "lua.hpp"
#include "player.hpp"
#include "server_entity_system.hpp"
#include "world.hpp"

using boost::format;
using namespace boost::math::constants;
namespace po = boost::program_options;

namespace hexa {

namespace {

template <class message_t>
message_t make (const packet& p)
{
    auto archive (make_deserializer(p));

    message_t new_msg;
    new_msg.serialize(archive);

    return new_msg;
}

} // anonymous namespace

//---------------------------------------------------------------------------

network::network(uint16_t port, world& w, server_entity_system& entities,
                 lua& scripting)
    : udp_server(port)
    , world_    (w)
    , es_       (entities)
    , lua_      (scripting)
    , running_  (false)
{
}

void network::run()
{
    running_.store(true);
    int count (0);

    while (true)
    {
        poll(5);

        // Send updated terrain
        for (chunk_coordinates c : world_.changeset)
            send_surface(c);

        world_.changeset.clear();

        for (map_coordinates c : world_.height_changeset)
            send_coarse_height(c);

        world_.height_changeset.clear();

        // Send changes in the entity system
        ++count;
        if (count % 20 == 0)
        {
            //trace("network tick");

            msg::entity_update_physics msg;
            auto lock (es_.acquire_read_lock());

            es_.for_each<wfpos, vector>(entity_system::c_position,
                                        entity_system::c_velocity,
                [&](es::storage::iterator i,
                    es::storage::var_ref<wfpos> p_,
                    es::storage::var_ref<vector> v_)
            {
                msg.updates.emplace_back(i->first, p_, v_);
            });

            auto n (clock::now());
            for (auto& c : connections_)
            {
                msg.timestamp = n - clock_offset_[c.second];
                send(c.second, serialize_packet(msg), msg.method());
            }

        }

        // Flush caches every now and then
        if (count % 207 == 0)
        {
            world_.cleanup();
        }

        while (!jobs.empty())
        {
            auto job (jobs.pop());

            trace((format("new network job type %1%") % job.type).str());

            switch (job.type)
            {
            case job::quit:
                running_.store(false);
                return;

            case job::lightmap:
                break;

            case job::surface_and_lightmap:
                {
                if (!world_.is_surface_available(job.pos))
                {
                    trace("cannot send surface %1%, data not available",
                          world_rel_coordinates(job.pos - world_chunk_center));
                    break;
                }
                auto conn (connections_.find(job.dest));
                if (conn == connections_.end())
                    break;

                send_surface(job.pos, conn->second);
                }
                break;

            case job::entity_info:
                break;
            }

            trace("network job finished");
        }
    }
}

void network::stop()
{
    jobs.push({ job::quit, chunk_coordinates(), 0 });
    while (running_.load())
        boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
}

void network::on_connect (ENetPeer* c)
{
    if (entities_.count(c))
    {
        trace("Player already connected wtf lol");
        return;
    }

    {
    auto write_lock (es_.acquire_write_lock());

    auto player_id (es_.new_entity());
    entities_[c] = player_id;
    connections_[player_id] = c;

    es_.set(player_id, server_entity_system::c_ip_addr, ip_address(c->address.host));

    log_msg("Player #%1% connected.", player_id);
    }

    // Greet the new player with the server name and our public key.
    msg::handshake m;

    m.server_name = "LOL server";
    m.public_key.resize(4);

    send(c, serialize_packet(m), m.method());

    {
    msg::define_resources msg;
    msg.textures.resize(texture_names.size());
    for (auto& rec : texture_names)
        msg.textures[rec.second] = rec.first;

    msg.models.push_back("mrfixit");

    send(c, serialize_packet(msg), msg.method());
    }

    {
    msg::define_materials msg;
    uint16_t index (0);
    for (const material& m : material_prop)
    {
        if (index != type::air && !m.name.empty())
            msg.materials.emplace_back(index, m);

        ++index;
    }
    send(c, serialize_packet(msg), msg.method());
    }

    clock_offset_[c] = clock::now();
}

void network::on_disconnect (ENetPeer* c)
{
    clock_offset_.erase(c);

    auto e (entities_.find(c));
    if (e == entities_.end())
    {
        log_msg("disconnect received from an unknown player");
        return;
    }
    auto entity_id (e->second);
    log_msg("disconnecting player %1%", entity_id);

    {
    auto write_lock (es_.acquire_write_lock());
    es_.delete_entity(entity_id);
    }

    connections_.erase(entity_id);
    entities_.erase(e);

    msg::entity_delete msg;
    msg.entity_id = entity_id;
    for (auto& conn : connections_)
        send(conn.second, serialize_packet(msg), msg.method());
}

void network::on_receive (ENetPeer* c, const packet& p)
{
    auto found (entities_.find(c));

    if (found == entities_.end())
    {
        // Ooo-kay this is spooky.  The player does not exist.  Drop the
        // connection straight away.
        ///\todo drop connection
        return;
    }

    try
    {
        packet_info info { found->second, c, p };

        switch(p.message_type())
        {
        case msg::login::msg_id:            login       (info);     break;
        case msg::logout::msg_id:           logout      (info);     break;
        case msg::time_sync_request::msg_id:timesync    (info);     break;
        case msg::request_heights::msg_id:  req_heights (info);     break;
        case msg::request_chunks::msg_id:   req_chunks  (info);     break;
        case msg::look_at::msg_id:          look_at     (info);     break;
        case msg::motion::msg_id:           motion      (info);     break;
        case msg::button_press::msg_id:     button_press(info);     break;
        case msg::button_release::msg_id:   button_release(info);   break;
        case msg::console::msg_id:          console     (info);     break;

        default:                            unknown     (info);
        }
    }
    catch (luabind::error& e)
    {
        log_msg("Lua error: %1%", lua_.get_error());
    }
    catch (std::exception& e)
    {
        log_msg("Could not handle packet: %1%", e.what());
    }
}

bool network::send (uint32_t entity, const std::vector<uint8_t>& msg,
                    msg::reliability method) const
{
    auto found (connections_.find(entity));
    bool have_connection (found != connections_.end());
    if (have_connection)
        send(found->second, msg, method);

    return have_connection;
}

/*

void network::change_block(const world_coordinates& pos, uint16_t material)
{
    chunk_index ci (pos % chunk_size);
    auto lock (acquire_write_lock());
    world::change_block(pos, material);

    chunk_coordinates cpos (pos / chunk_size);

    if (material == 0)
    {
        if (ci.x == 0)
            send_surface(cpos + chunk_coordinates(-1, 0, 0));
        else if (ci.x == chunk_size - 1)
            send_surface(cpos + chunk_coordinates(1, 0, 0));

        if (ci.y == 0)
            send_surface(cpos + chunk_coordinates(0, -1, 0));
        else if (ci.y == chunk_size - 1)
            send_surface(cpos + chunk_coordinates(0, 1, 0));

        if (ci.z == 0)
            send_surface(cpos + chunk_coordinates(0, 0, -1));
        else if (ci.z == chunk_size - 1)
            send_surface(cpos + chunk_coordinates(0, 0, 1));
    }

    send_surface(cpos);
}

void network::change_block(const world_coordinates& pos, const std::string& material)
{
    change_block(pos, find_material(material));
}
*/

void network::send_surface(const chunk_coordinates& cpos)
{
    trace("broadcast surface %1%", world_vector(cpos - world_chunk_center));

    msg::surface_update reply;
    reply.position = cpos;
    reply.terrain  = world_.get_compressed_surface(cpos);
    reply.light    = world_.get_compressed_lightmap(cpos);

    assert(count_faces(deserialize_as<surface_data>(decompress(reply.terrain)).opaque) == world_.get_lightmap(cpos)->opaque.size());
    assert(count_faces(deserialize_as<surface_data>(decompress(reply.terrain)).transparent) == world_.get_lightmap(cpos)->transparent.size());

    for (auto& conn : connections_)
    {
        auto plr_pos (es_.get<wfpos>(conn.first, entity_system::c_position));
        auto dist (manhattan_distance(cpos, plr_pos.pos / chunk_size));
        if (dist < 64)
            send(conn.second, serialize_packet(reply), reply.method());
    }
}

void network::send_surface(const chunk_coordinates& cpos, uint32_t dest)
{
    trace("new job: surface %1%", world_vector(cpos - world_chunk_center));
    jobs.push({ job::surface_and_lightmap, cpos, dest });
}

void network::send_surface(const chunk_coordinates& cpos, ENetPeer* dest)
{
    trace("send surface %1%", world_vector(cpos - world_chunk_center));

    msg::surface_update reply;
    reply.position = cpos;
    reply.terrain  = world_.get_compressed_surface(cpos);
    reply.light    = world_.get_compressed_lightmap(cpos);

    assert(deserialize_as<surface_data>(decompress(reply.terrain)).opaque == world_.get_surface(cpos)->opaque);
    assert(world_.get_lightmap(cpos) != nullptr);
    assert(deserialize_as<light_data>(decompress(reply.light)).opaque == world_.get_lightmap(cpos)->opaque);
    assert(count_faces(deserialize_as<surface_data>(decompress(reply.terrain)).opaque) == world_.get_lightmap(cpos)->opaque.size());
    assert(deserialize_as<surface_data>(decompress(reply.terrain)).transparent == world_.get_surface(cpos)->transparent);
    assert(count_faces(deserialize_as<surface_data>(decompress(reply.terrain)).transparent) == world_.get_lightmap(cpos)->transparent.size());

    send(dest, serialize_packet(reply), reply.method());
}

void network::send_coarse_height(const map_coordinates& mpos)
{
    auto height (world_.get_coarse_height(mpos));
    if (height == undefined_height)
        return;

    log_msg("broadcast heightmap %1%", map_rel_coordinates(mpos - map_chunk_center));

    msg::heightmap_update heights;
    heights.data.emplace_back(mpos, height);

    for (auto& conn : connections_)
    {
        send(conn.second, serialize_packet(heights), heights.method());
    }
    log_msg("broadcast heightmap %1% done", map_rel_coordinates(mpos - map_chunk_center));
}

void network::send_height(const map_coordinates& cpos, ENetPeer* dest)
{
    auto height (world_.get_coarse_height(cpos));
    if (height == undefined_height)
        return;

    trace("send height %1%", map_rel_coordinates(cpos - map_chunk_center));
    msg::heightmap_update heights;
    heights.data.emplace_back(cpos, height);
    send(dest, serialize_packet(heights), heights.method());
}

void network::login (const packet_info& info)
{
    auto msg (make<msg::login>(info.p));

    log_msg("player %1% login", info.plr);
    world_coordinates start_pos (world_center);

    // Move the spawn point to the lowlands.
    int hm (world_.find_area_generator("heightmap"));
    if (hm != -1)
    {
        for(;;)
        {
            area_ptr ap (world_.get_area_data(start_pos / chunk_size, hm));
            int16_t local_height((*ap)(8, 8));
            if (   local_height > 10 && local_height < 400)
            {
                start_pos.z = local_height + water_level + 4;
                break;
            }
            else
            {
                start_pos.x += chunk_size;
            }
        }
    }
    else
    {
        auto ch (world_.get_coarse_height(start_pos / chunk_size));
        if (ch != undefined_height)
            start_pos.z = ch * chunk_size;
        else
            start_pos.z = water_level + 400;
    }

    // Move the spawn point to the surface.
    if (world_.get_block(start_pos + dir_vector[dir_down]) == type::air)
    {
        do
        {
            start_pos.z -=2;
        }
        while (world_.get_block(start_pos + dir_vector[dir_down]) == type::air);
    }
    else
    {
        do
        {
            start_pos.z += 2;
        }
        while (world_.get_block(start_pos) != type::air) ;
    }

    start_pos.z += 12;
    log_msg("Spawning new player at %1%", start_pos);
    wfpos start_pos_sub (start_pos, vector(0.5, 0.5, 0.5));
    {
    auto write_lock (es_.acquire_write_lock());

    es_.set(info.plr, server_entity_system::c_name, msg.username);
    es_.set(info.plr, server_entity_system::c_position, start_pos_sub);
    es_.set(info.plr, server_entity_system::c_velocity, vector(0,0,0));
    es_.set(info.plr, server_entity_system::c_boundingbox, vector(0.4,0.4,1.7));
    es_.set(info.plr, server_entity_system::c_lookat, yaw_pitch(0, 0));
    }

    // Log in
    log_msg("send greeting to player %1%", info.plr);

    msg::greeting reply;
    reply.position = start_pos;
    reply.entity_id = info.plr;
    reply.client_time = clock::client_time(clock_offset_[info.conn]);
    reply.motd = "Be excellent to eachother.";
    send(info.conn, serialize_packet(reply), reply.method());

    // Send height maps
    log_msg("send height maps to player %1%", info.plr);
    chunk_coordinates pcp (start_pos / chunk_size);
    int hmr (12);
    msg::heightmap_update heights;
    heights.data.reserve((hmr + 1) * (hmr + 1));

    //trace("   trying to get read lock...");
    //auto lock (acquire_read_lock());
    //trace("   got lock");

    for (uint32_t y (pcp.y - hmr); y <= pcp.y + hmr; ++y)
    {
        for (uint32_t x (pcp.x - hmr); x <= pcp.x + hmr; ++x)
        {
            map_coordinates mc (x, y);
            auto height (world_.get_coarse_height(mc));
            if (height != undefined_height)
                heights.data.emplace_back(mc, height);
        }
    }
    send(info.conn, serialize_packet(heights), heights.method());

    log_msg("send terrain to player %1%", info.plr);

    // Send the surrounding terrain

    // If the player starts high above ground, send the first normal
    // terrain chunk below instead.
    //
    auto ch (world_.get_coarse_height(pcp));
    if (is_air_chunk(pcp, ch))
        pcp.z = ch - 1;

    world_.requests.push({ world::request::surface_and_lightmap, pcp,
                           [=]{ send_surface(pcp, info.plr); }
                           });

/*
    try
    {
        msg::surface_update reply;
        reply.position = pcp;

        // If the player starts high above ground, send the first normal
        // terrain chunk below instead.
        //
        auto ch (world_.get_coarse_height(pcp));
        if (is_air_chunk(pcp, ch))
            pcp.z = ch - 1;

        trace((boost::format("send chunk %1%") % world_vector(pcp - world_chunk_center)).str());

        reply.terrain = world_.get_compressed_surface(pcp);
        reply.light   = world_.get_compressed_lightmap(pcp);

        send(info.conn, serialize_packet(reply), reply.method());
    }
    catch (std::exception& e)
    {
        std::cout << "msg::req_chunks: cannot provide " << pcp << " to player, because: " << e.what() << std::endl;
    }
*/

    log_msg("send position to player %1%", info.plr);

    // Send the position to the player
    msg::entity_update posmsg;
    msg::entity_update::value rec;

    rec.entity_id = info.plr;
    rec.component_id = entity_system::c_position;
    rec.data = serialize_c(start_pos_sub);
    posmsg.updates.push_back(rec);

    rec.component_id = entity_system::c_boundingbox;
    rec.data = serialize_c(vector(0.4,0.4,1.7));
    posmsg.updates.push_back(rec);

    rec.component_id = entity_system::c_lookat;
    rec.data = serialize_c(yaw_pitch(0, 0));
    posmsg.updates.push_back(rec);

    rec.component_id = entity_system::c_velocity;
    rec.data = serialize_c(vector(0, 0, 0));
    posmsg.updates.push_back(rec);

    for (auto& conn : connections_)
    {
        if (conn.first == info.plr)
            continue;

        log_msg("inform player %1% of player %2%", conn.first, info.plr);
        send(conn.second, serialize_packet(posmsg), msg::reliable);
    }

    es_.for_each<wfpos>(entity_system::c_position,
        [&](es::storage::iterator i,
            es::storage::var_ref<wfpos> p_)
    {
        if (info.plr == i->first)
            return;

        log_msg("inform player %1% of player %2%", info.plr, i->first);
        rec.entity_id = i->first;
        rec.component_id = entity_system::c_boundingbox;
        rec.data = serialize_c(vector(0.4,0.4,1.7));
        posmsg.updates.push_back(rec);
        rec.component_id = entity_system::c_position;
        rec.data = serialize_c(wfpos(p_));
        posmsg.updates.push_back(rec);
        rec.component_id = entity_system::c_velocity;
        rec.data = serialize_c(vector(0, 0, 0));
        posmsg.updates.push_back(rec);
    });

    send(info.conn, serialize_packet(posmsg), msg::reliable);

    try
    {
        lua_.player_logged_in(info.plr);
    }
    catch (luabind::error& e)
    {
        log_msg("Lua error while logging in: %1%", lua_.get_error());
    }
    catch (std::exception& e)
    {
        log_msg("Error while logging in: %1%", e.what());
    }
    catch (...)
    {
        log_msg("Unknown error while logging in.");
    }

    log_msg("player %1% is logged in", info.plr);
}

void network::logout (const packet_info& info)
{
    log_msg("player %1% logout", info.plr);
}

void network::timesync (const packet_info& info)
{
    auto msg (make<msg::time_sync_request>(info.p));

    msg::time_sync_response answer;
    answer.request = msg.request;
    answer.response = clock::client_time(clock_offset_[info.conn]);

    send(info.conn, serialize_packet(answer), answer.method());
}

void network::req_heights (const packet_info& info)
{
    auto msg (make<msg::request_heights>(info.p));
    msg::heightmap_update answer;
    answer.data.reserve(msg.requests.size());

    for (auto& req : msg.requests)
    {
        try
        {
            trace("requesting height at %1%", req.position);

            auto height (world_.get_coarse_height(req.position));
            if (height != undefined_height)
                answer.data.emplace_back(req.position, height);
        }
        catch (std::exception& e)
        {
            trace("cannot provide height at %1%, because: %2%",
                  req.position, std::string(e.what()));
        }
    }

    send(info.conn, serialize_packet(answer), answer.method());
}

void network::req_chunks (const packet_info& info)
{
    auto msg (make<msg::request_chunks>(info.p));

    for(auto& req : msg.requests)
    {
        trace("request for surface %1%", world_rel_coordinates(req.position - world_chunk_center));
        try
        {
            if (is_air_chunk(req.position, world_.get_coarse_height(req.position)))
            {
                trace("air chunk, sending coarse height");
                send_height(req.position, info.conn);
                continue;
            }

            bool chunk_ok (world_.is_chunk_available(req.position));
            bool light_ok (world_.is_lightmap_available(req.position));

            // If all the data we need is available, send it immediately.
            // Otherwise, send a job down the queue and ask the terrain
            // generator to call us back when it's done.
            //
            if (chunk_ok && light_ok)
            {
                trace("sending surface right away");
                send_surface(req.position, info.conn);
            }
            else if (chunk_ok && !light_ok)
            {
                trace("have surface, generate lightmap");
                world_.requests.push({ world::request::lightmap,
                                       req.position,
                                       [=]{ send_surface(req.position, info.plr); }
                                       });
            }
            else
            {
                trace("generate surface and lightmap");
                world_.requests.push({ world::request::surface_and_lightmap,
                                       req.position,
                                       [=]{ send_surface(req.position, info.plr); }
                                       });
            }
        }
        catch (std::exception& e)
        {
            log_msg("Cannot provide surface data at %1%, because: %2%",
                  req.position, std::string(e.what()));
        }
    }
}

void network::motion (const packet_info& info)
{
    auto write_lock (es_.acquire_write_lock());
    auto msg (make<msg::motion>(info.p));

    float angle ((float)msg.move_dir / 256.f * two_pi<float>());
    vector2<float> move (from_polar(angle));

    const float walk_force (40.0f);
    float magnitude (walk_force * (float)msg.move_speed / 255.f);

    trace("player %1% moves in direction %2%", info.plr, move);
    es_.set(info.plr, entity_system::c_walk, move * magnitude);

    constexpr float lag (0.05);
    auto p (es_.get<wfpos>(info.plr, entity_system::c_position));
    auto v (es_.get<vector>(info.plr, entity_system::c_velocity));
    auto l (es_.get<yaw_pitch>(info.plr, entity_system::c_lookat));

    v += rotate(move, -l.x) * lag;
    p += v * lag;
    // hack
    p = msg.position;

    es_.set(info.plr, entity_system::c_position, p);
    es_.set(info.plr, entity_system::c_velocity, v);
}

void network::look_at (const packet_info& info)
{
    auto write_lock (es_.acquire_write_lock());
    auto msg (make<msg::look_at>(info.p));
    es_.set(info.plr, entity_system::c_lookat, msg.look);
}

void network::button_press (const packet_info& info)
{
    auto msg (make<msg::button_press>(info.p));
    //es_.set(info.plr, entity_system::c_position, msg.pos);
    //es_.set(info.plr, entity_system::c_lookat, msg.look);
    lua_.start_action(info.plr, msg.button, msg.slot, msg.look, msg.pos);
}

void network::button_release (const packet_info& info)
{
    auto msg (make<msg::button_release>(info.p));
    lua_.stop_action(info.plr, msg.button);
}

void network::console (const packet_info& info)
{
    auto msg (make<msg::console>(info.p));
    lua_.console(info.plr, msg.text);
}

void network::unknown (const packet_info& info)
{
    log_msg("Unknown packet type %1% received", (int)info.p.message_type());
}

void network::tick()
{
    using namespace std::chrono;
/*
    static size_t count (0);
    auto last_tick (steady_clock::now());

    while(!stop_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ++count;

        auto current_time (steady_clock::now());
        auto delta (current_time - last_tick);
        last_tick = current_time;
        double seconds (duration_cast<microseconds>(delta).count() * 1.0e-6);

        if (!requests_.empty())
        {
        auto& r (requests_.front());
        try
        {
            trace((boost::format("requesting surface at %1%") % world_vector(r.first - world_chunk_center)).str());

            if (!is_air_chunk(r.first, get_coarse_height(r.first)))
            {
                msg::surface_update reply;
                reply.position = r.first;
                reply.terrain  = get_compressed_surface(r.first);
                reply.light    = get_compressed_lightmap(r.first);

                auto tmp (get_surface(r.first));
                if (reply.light.empty() && !tmp->empty())
                    std::cout << "network::req_chunks: surface without light " << r.first << std::endl;

                send(r.second, serialize_packet(reply), reply.method());
            }
            else
            {
                trace("skipping because it's an air chunk");
            }
        }
        catch (std::exception& e)
        {
            std::cout << "msg::req_chunks: cannot provide " << r.first
                      << " to player, because: " << e.what() << std::endl;
        }
        requests_.pop_front();
        }

        // Every two seconds we do some background work, such as writing
        // dirty chunks back to file, throwing old chunks out of the
        // memory cache, etc.

        if (count % 20 == 0)
        {
            trace("acquire write lock...");
            auto lock (acquire_write_lock());
            trace("got lock, cleaning up...");
            cleanup();
            trace("done cleaning up");
        }

        {
        boost::mutex::scoped_lock lock (entities_mutex_);
        system_motion(entities_, seconds);

        if (count % 3 == 0)
        {
            msg::entity_update lol;

            entities_.for_each<wfpos, vector>(c_position, c_velocity,
                [&](es::storage::iterator i,
                    es::storage::var_ref<wfpos> a,
                    es::storage::var_ref<vector> b)
            {
                msg::entity_update::value rec;

                rec.entity_id = i->first;
                rec.component_id = c_position;
                rec.value = serialize_c(static_cast<wfpos>(a));
                lol.updates.push_back(rec);

                rec.component_id = c_velocity;
                rec.value = serialize_c(static_cast<vector>(b));
                lol.updates.push_back(rec);
            });

            for (auto& conn : players_)
            {
                send(conn.first, serialize_packet(lol), lol.method());
            }
        }
        }
    }
    */
}

} // namespace hexa

