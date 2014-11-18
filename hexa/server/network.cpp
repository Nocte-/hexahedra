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
// Copyright 2012-2014, nocte@hippie.nu
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
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <hexa/base58.hpp>
#include <hexa/compression.hpp>
#include <hexa/entity_system.hpp>
#include <hexa/entity_system_physics.hpp>
#include <hexa/geometric.hpp>
#include <hexa/log.hpp>
#include <hexa/protocol.hpp>
#include <hexa/rest.hpp>
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
using namespace boost::chrono;
using namespace boost::property_tree;
namespace po = boost::program_options;

namespace hexa
{

extern po::variables_map global_settings;

namespace
{

template <class message_t>
message_t make(const packet& p)
{
    auto archive(make_deserializer(p));

    message_t new_msg;
    new_msg.serialize(archive);

    return new_msg;
}

} // anonymous namespace

//---------------------------------------------------------------------------

network::network(uint16_t port, world& w, server_entity_system& entities,
                 lua& scripting)
    : udp_server(port)
    , world_(w)
    , es_(entities)
    , lua_(scripting)
    , running_(false)
{
    world_.on_update_surface.connect(
        [&](chunk_coordinates pos) { on_update_surface(pos); });

    world_.on_update_coarse_height.connect(
        [&](chunk_coordinates pos) { send_coarse_height(pos); });
}

network::~network()
{
    // world_.store(es_);
}

void network::run(const crypto::private_key& privkey,
                  const crypto::buffer& server_id)
{
    my_private_key_ = privkey;
    my_public_key_ = crypto::to_binary(crypto::get_public_key(privkey));
    my_id_ = server_id;
    log_msg("Network running, server ID %1%", base58_encode(my_id_));

    running_.store(true);
    int count(0);
    // auto last_tick (steady_clock::now());
    // auto started (last_tick);

    while (true) {
        poll(1);
        /*
                auto current_time (steady_clock::now());
                auto delta (current_time - last_tick);
                last_tick = current_time;
                auto delta_seconds (duration_cast<microseconds>(delta).count()
           * 1.0e-6);

                auto total_passed (current_time - started);
                auto total_seconds
           (duration_cast<microseconds>(total_passed).count() * 1.0e-6);
        */
        /*
        {
            boost::lock_guard<boost::mutex> lock(world_.changeset_lock);
            for (chunk_coordinates c : world_.changeset)
                send_surface(c);

            world_.changeset.clear();
        }

        {
            boost::lock_guard<boost::mutex> lock(world_.height_changeset_lock);
            for (map_coordinates c : world_.height_changeset)
                send_coarse_height(c);

            world_.height_changeset.clear();
        }
*/
        // Send changes in the entity system
        ++count;
        // count = total_seconds * 20;
        if (count % 200 == 0) {
            // trace("network tick");

            msg::entity_update_physics msg;
            auto lock(es_.acquire_read_lock());

            es_.for_each<wfpos, vector>(
                entity_system::c_position, entity_system::c_velocity,
                [&](es::storage::iterator i, wfpos& p_, vector& v_) {
                    msg.updates.emplace_back(i->first, p_, v_);
                    return false;
                });

            auto n(clock::now());
            for (auto& c : connections_) {
                msg.timestamp = n - conn_info_[c.second].clock_offset;
                send(c.second, serialize_packet(msg), msg.method());
            }
        }

        if (count % 899 == 0) {
            auto lock(es_.acquire_read_lock());
            for (auto i(es_.begin()); i != es_.end(); ++i) {
                if (es_.check_dirty(i)) {
                    msg::entity_update upd_msg;

                    if (es_.entity_has_component(i, entity_system::c_hotbar)) {
                        auto hb(es_.get<hotbar>(i, entity_system::c_hotbar));
                        binary_data blob(serialize(hb));
                        msg::entity_update::value val(
                            i->first, (uint16_t)entity_system::c_hotbar,
                            std::move(blob));
                        upd_msg.updates.emplace_back(std::move(val));
                    }
                    auto conn(connections_.find(i->first));
                    if (conn != connections_.end())
                        send(conn->first, serialize_packet(upd_msg),
                             upd_msg.method());
                }
            }
        }

        // Flush caches every now and then
        if (count % 2077 == 0) {
            world_.cleanup();
        }

        while (!jobs.empty()) {
            auto job(jobs.pop());

            trace((format("new network job type %1%") % job.type).str());

            switch (job.type) {
            case job::quit:
                running_.store(false);
                return;

            case job::lightmap:
                break;

            case job::surface_and_lightmap:
                send_surface(job.pos, job.dest);
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
    jobs.push({job::quit, chunk_coordinates(), nullptr});
    while (running_.load())
        boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
}

void network::on_connect(ENetPeer* c)
{
    if (conn_info_.count(c)) {
        log_msg("Player already connected");
        return;
    }
    log_msg("New player connected.");

    // Greet the new player with the server name and our public key.
    msg::handshake m;

    m.server_name = "LOL server";
    m.server_id = my_id_;
    m.public_key = my_public_key_;
    m.nonce = conn_info_[c].iv = crypto::make_random(16);
    send(c, serialize_packet(m), m.method());

    // Send resources, material list, etc.
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
        uint16_t index(0);
        for (const material& m : material_prop) {
            if (index != type::air && !m.name.empty())
                msg.materials.emplace_back(index, m);

            ++index;
        }
        send(c, serialize_packet(msg), msg.method());
    }

    conn_info_[c].clock_offset = clock::now();
}

void network::on_disconnect(ENetPeer* c)
{
    auto& info = conn_info_[c];
    log_msg("Disconnect player %1%", info.entity);
    deactivate_player(info.entity);
    connections_.erase(info.entity);
    conn_info_.erase(c);

    msg::entity_delete msg;
    msg.entity_id = info.entity;
    auto packet = serialize_packet(msg);
    for (auto& conn : connections_)
        send(conn.second, packet, msg.method());
}

void network::on_receive(ENetPeer* c, packet p)
{
    es::entity ent = 0;
    auto found = conn_info_.find(c);
    if (found != conn_info_.end())
        ent = found->second.entity;

    if (p.is_encrypted()) {
        if (found == conn_info_.end() || found->second.iv.empty()) {
            log_msg("Cannot decrypt packet, ignoring");
            return;
        }
        auto& info = found->second;
        p.decrypt(info.iv, info.cipher);
    }

    try {
        packet_info info{ent, c, p};

        switch (p.message_type()) {
        case msg::login::msg_id:
            login(info);
            break;
        case msg::logout::msg_id:
            logout(info);
            break;
        case msg::time_sync_request::msg_id:
            timesync(info);
            break;
        case msg::request_heights::msg_id:
            req_heights(info);
            break;
        case msg::request_surfaces::msg_id:
            req_chunks(info);
            break;
        case msg::look_at::msg_id:
            look_at(info);
            break;
        case msg::motion::msg_id:
            motion(info);
            break;
        case msg::button_press::msg_id:
            button_press(info);
            break;
        case msg::button_release::msg_id:
            button_release(info);
            break;
        case msg::console::msg_id:
            console(info);
            break;

        default:
            unknown(info);
        }
    } catch (luabind::error&) {
        log_msg("Lua error: %1%", lua_.get_error());
    } catch (std::exception& e) {
        log_msg("Could not handle packet type %1%: %2%", (int)p.message_type(),
                e.what());
    }
}

void network::send_encrypted(ENetPeer* dest, const binary_data& msg,
                             msg::reliability method) const
{
    auto found_info = conn_info_.find(dest);
    if (found_info == conn_info_.end()
        || !found_info->second.cipher.is_ready()) {
        // No encryption required, send straight away.
        send(dest, msg, method);
    } else {
        // Encrypt it before sending.
        auto& info = found_info->second;
        binary_data encrypt(msg.size() + 5);
        encrypt[0] = 0xff;
        uint32_t timer = clock::client_time(info.clock_offset);
        *(reinterpret_cast<uint32_t*>(&encrypt[1])) = timer;

        crypto::buffer iv = info.iv;
        *reinterpret_cast<uint32_t*>(&iv[0]) ^= timer;

        info.cipher.encrypt(iv, &msg[0], msg.size(), &encrypt[5]);
        send(dest, encrypt, method);
    }
}

bool network::send(uint32_t entity, const binary_data& msg,
                   msg::reliability method) const
{
    auto found = connections_.find(entity);
    bool have_connection = (found != connections_.end());
    if (have_connection)
        send_encrypted(found->second, msg, method);

    return have_connection;
}

void network::broadcast(const binary_data& msg, msg::reliability method) const
{
    for (auto& conn : connections_)
        send_encrypted(conn.second, msg, method);
}

void network::send_surface(const chunk_coordinates& cpos)
{
    trace("broadcast surface %1%", world_vector(cpos - world_chunk_center));
    auto proxy(world_.acquire_read_access());

    msg::surface_update reply;
    reply.position = cpos;
    reply.terrain = proxy.get_compressed_surface(cpos);
    reply.light = proxy.get_compressed_lightmap(cpos);

    for (auto& conn : connections_) {
        auto plr_pos(es_.get<wfpos>(conn.first, entity_system::c_position));
        auto dist(manhattan_distance(cpos, plr_pos.pos / chunk_size));
        if (dist < 64)
            send(conn.second, serialize_packet(reply), reply.method());
    }
}

void network::send_surface_queue(const chunk_coordinates& cpos, ENetPeer* dest)
{
    trace("new job: surface %1%", world_vector(cpos - world_chunk_center));
    jobs.push({job::surface_and_lightmap, cpos, dest});
}

void network::send_surface(const chunk_coordinates& cpos, ENetPeer* dest)
{
    trace("send surface %1%", world_vector(cpos - world_chunk_center));
    auto proxy(world_.acquire_read_access());

    msg::surface_update reply;
    reply.position = cpos;
    reply.terrain = proxy.get_compressed_surface(cpos);
    reply.light = proxy.get_compressed_lightmap(cpos);

    send(dest, serialize_packet(reply), reply.method());
    trace("send surface %1% done", world_vector(cpos - world_chunk_center));
}

void network::send_coarse_height(chunk_coordinates pos)
{
    trace("broadcast heightmap %1%",
          map_rel_coordinates(pos - map_chunk_center));

    msg::heightmap_update heights;
    heights.data.emplace_back(pos, pos.z);

    for (auto& conn : connections_) {
        send(conn.second, serialize_packet(heights), heights.method());
    }
    trace("broadcast heightmap %1% done",
          map_rel_coordinates(pos - map_chunk_center));
}

void network::send_height(const map_coordinates& cpos, ENetPeer* dest)
{
    auto height(coarse_height(world_, cpos));
    if (height == undefined_height)
        return;

    trace("send height %1%", map_rel_coordinates(cpos - map_chunk_center));
    msg::heightmap_update heights;
    heights.data.emplace_back(cpos, height);
    send(dest, serialize_packet(heights), heights.method());
}

void network::kick_player(ENetPeer* dest, const std::string& kickmsg)
{
    log_msg("Kick player: %1%", kickmsg);
    msg::kick reply;
    reply.reason = kickmsg;
    send_encrypted(dest, serialize_packet(reply), reply.method());
    disconnect(dest);
    on_disconnect(dest);
}

void network::login(packet_info& info)
{
    using namespace crypto;

    auto msg = make<msg::login>(info.p);

    world_coordinates start_pos(world_center);
    wfpos start_pos_sub;
    auto player_name = msg.name;
    bool hashed_id = false;

    // If no player ID was given, generate one by hashing the client's
    // IP address.
    if (msg.uid.empty()) {
        auto& addr = info.conn->address.host;
        auto ptr = reinterpret_cast<uint8_t*>(&addr);
        crypto::buffer b(ptr, ptr + sizeof(addr));
        auto hash = sha256(b);
        hash.resize(8);
        std::copy(hash.begin(), hash.end(), std::back_inserter(msg.uid));
        hashed_id = true;
    }

    if (msg.uid.size() != 8) {
        kick_player(info.conn, "No valid player ID was given");
        return;
    }

    log_msg("player '%1%' tries to login", player_name);
    auto& cinfo = conn_info_[info.conn];

    if (msg.mode == 0) {
        // Localhost singleplayer mode
        if (global_settings["mode"].as<std::string>() != "singleplayer") {
            kick_player(info.conn, "Server is not in singleplayer mode");
            return;
        }
        info.plr = 0;

    } else if (msg.mode == 1) {
        // Multiplayer mode
        try {
            if (global_settings["mode"].as<std::string>() == "singleplayer")
                throw "Server is not running in multiplayer mode";

            if (msg.public_key.size() != 33)
                throw "No valid UID or public key";

            auto their_pubkey = public_key_from_binary(msg.public_key);
            if (!crypto::is_valid(their_pubkey))
                throw "Public key is not valid";

            auto shared_secret = crypto::ecdh(their_pubkey, my_private_key_);
            // The shared secret is too wide for our purposes, use only the
            // lowest 128 bits.
            shared_secret.erase(shared_secret.begin() + 16,
                                shared_secret.end());
            cinfo.cipher.set_key(x_or(shared_secret, cinfo.iv));

            if (sha256(concat(shared_secret, cinfo.iv)) != msg.mac) {
                throw "Could not authenticate player";
            } else {
                log_msg("MAC is OK");
            }

            if (!hashed_id) {
                // This is a registered user ID, run a few checks first.
                std::string url{"https://auth.hexahedra.net/api/1/users/"};
                auto res = rest::get(url + base58_encode(msg.uid));
                if (res.status_code == 200) {
                    if (from_json(res.json.get_child("users.pubkey")) != their_pubkey)
                        throw "Could not authenticate player";

                    player_name = res.json.get("users.username", player_name);
                } else if (res.status_code == 404) {
                    throw "Player ID does not exist";
                }
            }

            if (player_name.empty()) {
                player_name = "Guest"
                              + std::to_string(
                                    fnv_hash(info.conn->address.host) % 1000);
            } else if (player_name.size() > 24) {
                throw "Player name is too long";
            }

            int found(0);
            es::storage::iterator iter;
            uint64_t player_uid
                = *reinterpret_cast<const uint64_t*>(&msg.uid[0]);

            es_.for_each<uint64_t>(
                server_entity_system::c_player_uid,
                [&](es::storage::iterator i, uint64_t& eid) {
                    trace("Check against %1%...", eid);
                    if (eid == player_uid) {
                        ++found;
                        iter = i;
                    }
                    return false;
                });

            if (found == 0) {
                trace("Log in new player with uid %1%", player_uid);
                info.plr = es_.new_entity();
                es_.set(info.plr, server_entity_system::c_player_uid,
                        player_uid);
            } else {
                trace("Log in existing player with uid %1%", player_uid);
                if (found > 1)
                    trace("ERROR: Found more than one, actually");

                info.plr = iter->first;
                reactivate_player(info.plr);
            }
        } catch (const char* msg) {
            kick_player(info.conn, msg);
            return;
        } catch (...) {
            kick_player(info.conn, "Cannot log in player");
            return;
        }
    } else {
        kick_player(info.conn, "Not a valid login method");
        return;
    }

    conn_info_[info.conn].entity = info.plr;
    connections_[info.plr] = info.conn;

    log_msg("player %1% (%2%) logged in", info.plr, player_name);

    auto pi = es_.make(info.plr);
    es_.set(pi, server_entity_system::c_name, player_name);

    if (es_.entity_has_component(pi, entity_system::c_position)) {
        start_pos_sub = es_.get<wfpos>(pi, entity_system::c_position);
    } else {
        // Move the spawn point to the lowlands.
        int hm(world_.find_area_generator("heightmap"));
        if (false && hm != -1) {
            size_t count(0);
            auto proxy(world_.acquire_read_access());
            for (;;) {
                auto& ap(proxy.get_area_data(start_pos / chunk_size, hm));
                int16_t local_height(ap(8, 8));
                if (++count > 100
                    || (local_height > 10 && local_height < 200)) {
                    start_pos.z = local_height + world_center.z + 4;
                    break;
                } else {
                    start_pos.x += chunk_size;
                }
            }
        } else {
            auto ch(coarse_height(world_, start_pos / chunk_size));
            if (ch != undefined_height && ch < chunk_world_limit.z)
                start_pos.z = ch * chunk_size;
            else
                start_pos.z = world_center.z + 4;
        }

        trace("Going to spawn player near %1%",
              world_rel_coordinates(start_pos - world_center));

        // Move the spawn point to the surface.
        if (false) { // temp. disabled
            auto proxy(world_.acquire_read_access());
            if (proxy.get_block(start_pos + dir_vector[dir_down])
                == type::air) {
                do {
                    trace("Moving down...");
                    start_pos.z -= 2;
                } while (proxy.get_block(start_pos + dir_vector[dir_down])
                         == type::air);
            } else {
                do {
                    trace("Moving up...");
                    start_pos.z += 2;
                } while (proxy.get_block(start_pos) != type::air);
            }
        }

        start_pos.z += 4;
        log_msg("Spawning new player at %1%", start_pos);
        trace("Final position: %1%",
              world_rel_coordinates(start_pos - world_center));

        start_pos_sub = wfpos(start_pos, vector(0.5, 0.5, 0.5));
        {
            auto write_lock = es_.acquire_write_lock();

            es_.set(info.plr, server_entity_system::c_position, start_pos_sub);
            es_.set(info.plr, server_entity_system::c_velocity,
                    vector(0, 0, 0));
            es_.set(info.plr, server_entity_system::c_boundingbox,
                    vector(0.4f, 0.4f, 1.73f));
            es_.set(info.plr, server_entity_system::c_lookat, yaw_pitch(0, 0));
        }
    }

    // Log in
    log_msg("send greeting to player %1%", info.plr);

    msg::greeting reply;
    reply.position = start_pos;
    reply.entity_id = info.plr;
    reply.client_time = clock::client_time(conn_info_[info.conn].clock_offset);
    reply.motd = "Be excellent to eachother.";
    send_encrypted(info.conn, serialize_packet(reply), reply.method());

    // Send height maps
    log_msg("send height maps to player %1%", info.plr);
    chunk_coordinates pcp(start_pos / chunk_size);
    int hmr(12);
    msg::heightmap_update heights;
    heights.data.reserve((hmr + 1) * (hmr + 1));

    for (uint32_t y(pcp.y - hmr); y <= pcp.y + hmr; ++y) {
        for (uint32_t x(pcp.x - hmr); x <= pcp.x + hmr; ++x) {
            map_coordinates mc(x, y);
            auto height(coarse_height(world_, mc));
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
    auto ch(coarse_height(world_, pcp));
    if (is_air_chunk(pcp, ch))
        pcp.z = ch - 1;

    trace("Request terrain %1% for player", pcp);
    workers_.enqueue([=] {
        prepare_for_player(world_, pcp);
        send_surface_queue(pcp, info.conn);
    });

    log_msg("send position to player %1%", info.plr);

    // Send the position to the player
    msg::entity_update posmsg;
    msg::entity_update::value rec;

    rec.entity_id = info.plr;
    rec.component_id = entity_system::c_position;
    rec.data = serialize_c(start_pos_sub);
    posmsg.updates.push_back(rec);

    rec.component_id = entity_system::c_boundingbox;
    rec.data = serialize_c(vector(0.4f, 0.4f, 1.7f));
    posmsg.updates.push_back(rec);

    rec.component_id = entity_system::c_lookat;
    rec.data = serialize_c(yaw_pitch(0, 0));
    posmsg.updates.push_back(rec);

    rec.component_id = entity_system::c_velocity;
    rec.data = serialize_c(vector(0, 0, 0));
    posmsg.updates.push_back(rec);

    for (auto& conn : connections_) {
        if (conn.first == info.plr)
            continue;

        log_msg("inform player %1% of player %2%", conn.first, info.plr);
        send_encrypted(conn.second, serialize_packet(posmsg), msg::reliable);
    }

    {
        auto read_lock(es_.acquire_read_lock());

        es_.for_each<wfpos>(entity_system::c_position,
                            [&](es::storage::iterator i, wfpos& p_) {
            if (info.plr == i->first)
                return false;

            log_msg("inform player %1% of player %2%", info.plr, i->first);
            rec.entity_id = i->first;
            rec.component_id = entity_system::c_boundingbox;
            rec.data = serialize_c(vector(0.4f, 0.4f, 1.7f));
            posmsg.updates.push_back(rec);
            rec.component_id = entity_system::c_position;
            rec.data = serialize_c(wfpos(p_));
            posmsg.updates.push_back(rec);
            rec.component_id = entity_system::c_velocity;
            rec.data = serialize_c(vector(0, 0, 0));
            posmsg.updates.push_back(rec);

            return false;
        });
    }

    send_encrypted(info.conn, serialize_packet(posmsg), msg::reliable);

    try {
        auto lock(es_.acquire_write_lock());
        lua_.player_logged_in(info.plr);
    } catch (luabind::error&) {
        log_msg("Lua error while logging in: %1%", lua_.get_error());
    } catch (std::exception& e) {
        log_msg("Error while logging in: %1%", e.what());
    } catch (...) {
        log_msg("Unknown error while logging in.");
    }

    log_msg("player %1% is logged in", info.plr);
}

void network::logout(const packet_info& info)
{
    log_msg("player %1% logout", info.plr);
}

void network::timesync(const packet_info& info)
{
    auto msg(make<msg::time_sync_request>(info.p));

    msg::time_sync_response answer;
    answer.request = msg.request;
    answer.response = clock::client_time(conn_info_[info.conn].clock_offset);

    send(info.conn, serialize_packet(answer), answer.method());
}

void network::req_heights(const packet_info& info)
{
    auto msg(make<msg::request_heights>(info.p));
    msg::heightmap_update answer;
    answer.data.reserve(msg.requests.size());

    for (auto& req : msg.requests) {
        try {
            trace("requesting height at %1%", req.position);

            auto height(coarse_height(world_, req.position));
            if (height != undefined_height)
                answer.data.emplace_back(req.position, height);
        } catch (std::exception& e) {
            (void)e;
            trace("cannot provide height at %1%, because: %2%", req.position,
                  std::string(e.what()));
        }
    }

    send(info.conn, serialize_packet(answer), answer.method());
}

void network::req_chunks(const packet_info& info)
{
    auto msg(make<msg::request_surfaces>(info.p));

    for (auto& req : msg.requests) {
        trace("request for surface %1%",
              world_rel_coordinates(req.position - world_chunk_center));
        try {
            auto pos(req.position);
            if (is_air_chunk(req.position, coarse_height(world_, pos))) {
                trace("air chunk, sending coarse height");
                send_height(pos, info.conn);
                continue;
            }

            bool chunk_ok;
            bool light_ok;

            {
                auto proxy(world_.acquire_read_access());

                if (proxy.is_surface_available(pos)
                    && req.version == proxy.get_surface(pos).version) {
                    continue;
                }

                chunk_ok = proxy.is_chunk_available(req.position);
                light_ok = proxy.is_lightmap_available(req.position);
            }

            // If all the data we need is available, send it immediately.
            // Otherwise, send a job down the queue and ask the terrain
            // generator to call us back when it's done.
            //
            if (chunk_ok && light_ok) {
                trace("sending surface right away");
                send_surface(req.position, info.conn);
            } else {
                trace("generate surface and lightmap");
                workers_.enqueue([=] {
                    prepare_for_player(world_, req.position);
                    send_surface_queue(req.position, info.conn);
                });
            }
        } catch (std::exception& e) {
            log_msg("Cannot provide surface data at %1%, because: %2%",
                    req.position, std::string(e.what()));
        }
    }
}

void network::motion(const packet_info& info)
{
    auto msg(make<msg::motion>(info.p));

    float angle((float)msg.move_dir / 256.f * two_pi<float>());
    vector2<float> move(from_polar(angle));

    const float walk_force = 1.0f;
    float magnitude(walk_force * (float)msg.move_speed / 255.f);

    trace("player %1% moves in direction %2%", info.plr, move);
    auto write_lock(es_.acquire_write_lock());
    es_.set_walk(info.plr, move * magnitude);

    constexpr float lag(0.05f);
    auto p = es_.get<wfpos>(info.plr, entity_system::c_position);
    auto v = es_.get<vector>(info.plr, entity_system::c_velocity);
    auto l = es_.get<yaw_pitch>(info.plr, entity_system::c_lookat);

    p += vector(rotate(move, -l.x), 0.0f) * 5.0f * lag;

    // hack
    p = msg.position;

    es_.set_position(info.plr, p);
    es_.set_velocity(info.plr, v);
}

void network::look_at(const packet_info& info)
{
    auto msg(make<msg::look_at>(info.p));
    auto write_lock(es_.acquire_write_lock());
    es_.set_lookat(info.plr, msg.look);
}

void network::button_press(const packet_info& info)
{
    auto msg(make<msg::button_press>(info.p));
    lua_.start_action(info.plr, msg.button, msg.slot, msg.look, msg.pos);
}

void network::button_release(const packet_info& info)
{
    auto msg(make<msg::button_release>(info.p));
    lua_.stop_action(info.plr, msg.button);
}

void network::console(const packet_info& info)
{
    auto msg(make<msg::console>(info.p));
    trace("Console msg: %1%", msg.text);
    lua_.console(info.plr, msg.text);
}

void network::unknown(const packet_info& info)
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
            double seconds (duration_cast<microseconds>(delta).count() *
       1.0e-6);

            if (!requests_.empty())
            {
            auto& r (requests_.front());
            try
            {
                trace((boost::format("requesting surface at %1%") %
       world_vector(r.first - world_chunk_center)).str());

                if (!is_air_chunk(r.first, get_coarse_height(r.first)))
                {
                    msg::surface_update reply;
                    reply.position = r.first;
                    reply.terrain  = get_compressed_surface(r.first);
                    reply.light    = get_compressed_lightmap(r.first);

                    auto tmp (get_surface(r.first));
                    if (reply.light.empty() && !tmp->empty())
                        std::cout << "network::req_chunks: surface without
       light " << r.first << std::endl;

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

void network::on_update_surface(const chunk_coordinates& pos)
{
    for (auto& conn : connections_)
        send_surface_queue(pos, conn.second);
}

bool network::deactivate_player(uint32_t entity)
{
    auto write_lock = es_.acquire_write_lock();
    auto i = es_.find(entity);
    if (i == es_.end()
        || !es_.entity_has_component(i, server_entity_system::c_position))
        return false;

    inactive_player data;
    data.saved_pos = es_.get<wfpos>(i, server_entity_system::c_position);
    data.logout = clock::epoch();
    es_.remove_component_from_entity(i, server_entity_system::c_position);
    es_.set(i, server_entity_system::c_inactive_player, data);

    log_msg("Player %1% has been deactivated", entity);

    return true;
}

bool network::reactivate_player(uint32_t entity)
{
    auto write_lock = es_.acquire_write_lock();
    auto i = es_.find(entity);
    if (i == es_.end()
        || !es_.entity_has_component(i,
                                     server_entity_system::c_inactive_player))
        return false;

    auto data
        = es_.get<inactive_player>(i, server_entity_system::c_inactive_player);
    es_.remove_component_from_entity(i,
                                     server_entity_system::c_inactive_player);
    es_.set(i, server_entity_system::c_position, data.saved_pos);

    log_msg("Player %1% has been reactivated", entity);

    return true;
}

} // namespace hexa
