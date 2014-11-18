//---------------------------------------------------------------------------
// udp_client.cpp
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

#include "udp_client.hpp"

#include <boost/format.hpp>
#include <boost/thread/locks.hpp>
#include <hexa/config.hpp>
#include <hexa/log.hpp>

using boost::format;

namespace hexa
{

udp_client::udp_client(std::string host, uint16_t port)
    : connected_(false)
{
    boost::lock_guard<boost::mutex> lock(host_mutex_);
    host_ = enet_host_create(nullptr, 1, UDP_CHANNELS, 0, 0);
    if (host_ == nullptr)
        throw network_error("could not set up UDP host");

    if (host.empty()) {
#ifdef ENET_IPV6
        host = "::1";
#else
        host = "127.0.0.1";
#endif
    }

    enet_address_set_host(&address_, host.c_str());
    address_.port = port;

    log_msg("udp_client set up peer to host %1%", host);
    peer_ = enet_host_connect(host_, &address_, UDP_CHANNELS, 0);
    if (peer_ == nullptr)
        throw network_error("could not set up UDP peer");
}

udp_client::~udp_client()
{
    disconnect();
    enet_host_destroy(host_);
}

bool udp_client::connect(unsigned int timeout)
{
    disconnect();
    ENetEvent ev;
    {
        boost::lock_guard<boost::mutex> lock(host_mutex_);
        if (enet_host_service(host_, &ev, timeout) > 0
            && ev.type == ENET_EVENT_TYPE_CONNECT) {
            log_msg("udp_client connected");
            connected_ = true;
            return true;
        }
    }

    log_msg("udp_client connect failed");
    enet_peer_reset(peer_);
    return false;
}

bool udp_client::disconnect()
{
    if (!is_connected())
        return false;

    ENetEvent ev;
    enet_peer_disconnect(peer_, 0);

    {
        boost::lock_guard<boost::mutex> lock(host_mutex_);
        while (enet_host_service(host_, &ev, 3000)) {
            switch (ev.type) {
            case ENET_EVENT_TYPE_DISCONNECT:
                connected_ = false;
                log_msg("udp_client disconnected");
                return true;

            default:
                enet_packet_destroy(ev.packet);
            }
        }
    }

    log_msg("udp_client disconnect failed");
    enet_peer_reset(peer_);
    return false;
}

void udp_client::poll(unsigned int milliseconds)
{
    ENetEvent ev;
    int result;
    {
        boost::lock_guard<boost::mutex> lock(host_mutex_);
        result = enet_host_service(host_, &ev, milliseconds);
    }

    if (result < 0)
        throw std::runtime_error(
            (format("network error %1%") % -result).str());

    switch (ev.type) {
    case ENET_EVENT_TYPE_CONNECT:
        on_connect();
        break;

    case ENET_EVENT_TYPE_RECEIVE:
        receive(packet(ev.packet->data, ev.packet->dataLength));
        break;

    case ENET_EVENT_TYPE_DISCONNECT:
        on_disconnect();
        break;

    case ENET_EVENT_TYPE_NONE:
        break;
    }

    if (ev.packet != nullptr)
        enet_packet_destroy(ev.packet);
}

bool udp_client::is_connected() const
{
    return connected_;
}

float udp_client::last_rtt() const
{
    return peer_->lastRoundTripTime * 0.001f;
}

float udp_client::rtt() const
{
    return peer_->roundTripTime * 0.001f;
}

void udp_client::send(const binary_data& p, msg::reliability method)
{
    uint32_t flags(0);

    switch (method) {
    case msg::unreliable:
        flags = ENET_PACKET_FLAG_UNSEQUENCED;
        break;
    case msg::reliable:
    case msg::sequenced:
        flags = ENET_PACKET_FLAG_RELIABLE;
        break;
    }

    ENetPacket* packet(enet_packet_create(&p[0], p.size(), flags));
    {
        boost::lock_guard<boost::mutex> lock(host_mutex_);
        auto res = enet_peer_send(peer_, 0, packet);
        if (res != 0)
            log_msg("udp_client send failed with code %1%", -res);
    }
}

} // namespace hexa
