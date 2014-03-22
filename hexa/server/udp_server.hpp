//---------------------------------------------------------------------------
/// \file   server/udp_server.hpp
/// \brief  Base class for UDP-based servers
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

#pragma once

#include <vector>
#include <boost/thread/mutex.hpp>
#include <enet/enet.h>
#include <hexa/protocol.hpp>

namespace hexa {

class udp_server
{
public:
    udp_server(uint16_t port, uint16_t max_users = 32);
    virtual ~udp_server();

    void poll (uint16_t milliseconds);

    void send (ENetPeer* dest, const std::vector<uint8_t>& msg,
               msg::reliability method) const;

    void broadcast (const std::vector<uint8_t>& msg,
                    msg::reliability method) const;

    virtual void on_connect (ENetPeer* peer) = 0;
    virtual void on_receive (ENetPeer* peer, const packet& pkt) = 0;
    virtual void on_disconnect (ENetPeer* peer) = 0;

private:
    ENetAddress             addr_;
    ENetHost*               sv_;
    mutable boost::mutex    enet_mutex_;
};

} // namespace hexa

