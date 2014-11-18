//---------------------------------------------------------------------------
/// \file   server/network.hpp
/// \brief  Handle network communcation with players
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
#pragma once

#include <atomic>
#include <tuple>
#include <unordered_map>

#include <boost/thread.hpp>

#include <es/entity.hpp>

#include <hexa/concurrent_queue.hpp>
#include <hexa/crypto.hpp>
#include <hexa/ray.hpp>
#include <hexa/threadpool.hpp>

#include "udp_server.hpp"
#include "player.hpp"

namespace hexa
{

class packet;
class server_entity_system;
class lua;
class world;

/** Callbacks for handling network events. */
class network : public udp_server
{
public:
    using udp_server::send;

public:
    struct job
    {
        enum type_t { lightmap, surface_and_lightmap, entity_info, quit };

        type_t type;
        chunk_coordinates pos;
        ENetPeer* dest;
    };

    concurrent_queue<job> jobs;

public:
    network(uint16_t port, world& storage, server_entity_system& entities,
            lua& scripting);

    ~network();

    void run(const crypto::private_key& privkey, const crypto::buffer& server_id);
    void stop();

    void on_connect(ENetPeer* c);
    void on_disconnect(ENetPeer* c);
    void on_receive(ENetPeer* c, packet p);

    bool send(uint32_t entity, const binary_data& msg,
              msg::reliability method) const;

    void send_encrypted(ENetPeer* dest, const binary_data& msg,
                        msg::reliability method) const;

    void broadcast(const binary_data& msg, msg::reliability method) const;

private:
    struct packet_info
    {
        es::entity plr;
        ENetPeer* conn;
        const packet& p;
    };

    void login(packet_info& p);
    void logout(const packet_info& p);
    void timesync(const packet_info& p);
    void req_heights(const packet_info& p);
    void req_chunks(const packet_info& p);
    void button_press(const packet_info& p);
    void button_release(const packet_info& p);
    void look_at(const packet_info& p);
    void motion(const packet_info& p);
    void console(const packet_info& p);
    void unknown(const packet_info& p);

private:
    void tick();
    void send_surface(const chunk_coordinates& pos);
    void send_surface_queue(const chunk_coordinates& pos, ENetPeer* dest);
    void send_surface(const chunk_coordinates& pos, ENetPeer* dest);
    void send_coarse_height(chunk_coordinates pos);
    void send_height(const map_coordinates& pos, ENetPeer* dest);
    void kick_player(ENetPeer* dest, const std::string& kickmsg);
    bool deactivate_player(uint32_t entity);
    bool reactivate_player(uint32_t entity);

    void on_update_surface(const chunk_coordinates& pos);

private:
    world& world_;
    server_entity_system& es_;
    lua& lua_;
    threadpool workers_;

    crypto::private_key my_private_key_;
    crypto::buffer my_public_key_;
    crypto::buffer my_id_;

    struct connection_info
    {
        uint64_t       clock_offset;
        uint32_t       entity;
        binary_data    iv;
        crypto::aes    cipher;
    };

    std::unordered_map<ENetPeer*, connection_info> conn_info_;

    //std::unordered_map<ENetPeer*, uint64_t> clock_offset_;
    //std::unordered_map<ENetPeer*, uint32_t> entities_;
    std::unordered_map<uint32_t, ENetPeer*> connections_;

    std::atomic<bool> running_;
};

} // namespace hexa
