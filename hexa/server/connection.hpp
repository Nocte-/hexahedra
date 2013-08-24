//---------------------------------------------------------------------------
/// \file   server/connection.hpp
/// \brief  A network connection with a client.
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

#include <enet/enet.h>
#include <hexa/packet.hpp>

namespace hexa {

/** */
class connection
{
public:
    connection(ENetPeer* peer) : peer_(peer), channel_jump_(0) {}

    uint8_t pick_channel() const
    {
        return 0;
    }

    uint32_t address() const { return peer_->address.host; }
    uint16_t port() const    { return peer_->address.port; }
    uint32_t ping() const    { return peer_->roundTripTime; }

private:
    ENetPeer*   peer_;
    uint8_t     channel_jump_;
};

/// Smart pointer for connections
typedef boost::shared_ptr<connection>  connection_ptr;

} // namespace hexa

