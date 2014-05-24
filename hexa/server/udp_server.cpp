//---------------------------------------------------------------------------
// server/udp_server.cpp
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

#include "udp_server.hpp"

#include <stdexcept>
#include <string>
#include <boost/format.hpp>
#include <boost/thread/locks.hpp>

using boost::format;

namespace hexa {

udp_server::udp_server(uint16_t port, uint16_t max_users)
    : sv_ (nullptr)
{
#ifdef ENET_IPV6
    addr_.host = in6addr_any;
#else
    addr_.host = ENET_HOST_ANY;
#endif
    addr_.port = port;

    sv_ = enet_host_create(&addr_, max_users, 3, 0, 0);
    if (!sv_)
        throw std::runtime_error((format("failed to open port %1% (do you already have a server running?)") % port).str());
}

udp_server::~udp_server()
{
    enet_host_destroy(sv_);
}

void udp_server::poll (uint16_t milliseconds)
{
    ENetEvent ev;
    int result = 0;
    {
    boost::lock_guard<boost::mutex> lock (enet_mutex_);
    result = enet_host_service(sv_, &ev, milliseconds);
    }

    if (result < 0)
        throw std::runtime_error((format("network error %1%") % -result).str());

    switch (ev.type)
    {
        case ENET_EVENT_TYPE_CONNECT:
            on_connect(ev.peer);
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            on_receive(ev.peer, packet(ev.packet->data, ev.packet->dataLength));
            enet_packet_destroy(ev.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            on_disconnect(ev.peer);
            break;

        case ENET_EVENT_TYPE_NONE:
            break;
    }
}

void udp_server::send (ENetPeer* peer, const binary_data& msg,
                       msg::reliability method) const
{
    uint32_t flags (0);

    switch (method)
    {
    case msg::unreliable: flags = ENET_PACKET_FLAG_UNSEQUENCED; break;
    case msg::reliable:
    case msg::sequenced:  flags = ENET_PACKET_FLAG_RELIABLE; break;
    }

    auto pkt (enet_packet_create(&msg[0], msg.size(), flags));
    {
    boost::lock_guard<boost::mutex> lock (enet_mutex_);
    enet_peer_send(peer, 0, pkt);
    }
}

void udp_server::disconnect (ENetPeer* peer)
{
    enet_peer_disconnect_now(peer, 0);
}

} // namespace hexa

