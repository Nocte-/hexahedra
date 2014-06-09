//---------------------------------------------------------------------------
/// \file   udp_client.hpp
/// \brief  Connect to an UDP server
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
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <stdexcept>
#include <boost/thread/mutex.hpp>
#include <enet/enet.h>
#include <hexa/packet.hpp>
#include <hexa/protocol.hpp>

namespace hexa
{

class network_error : public std::runtime_error
{
public:
    network_error(const std::string& what)
        : std::runtime_error(what.c_str())
    {
    }
};

class udp_client
{
public:
    udp_client(std::string host, uint16_t port);
    ~udp_client();
    udp_client(const udp_client&) = delete;

    bool connect(unsigned int timeout = 4000);

    bool disconnect();

    void poll(unsigned int timeout = 200);

    void send(const binary_data& p, msg::reliability method);

    virtual void on_connect() {}
    virtual void on_disconnect() {}
    virtual void receive(const packet& p) = 0;

    bool is_connected() const;

    /// Last measured round-trip time, in seconds.
    float last_rtt() const;

    /// Mean round-trip time, in seconds.
    float rtt() const;

protected:
    boost::mutex host_mutex_;
    ENetHost* host_;
    ENetAddress address_;
    ENetPeer* peer_;
    bool connected_;
};

} // namespace hexa
