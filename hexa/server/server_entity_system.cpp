//---------------------------------------------------------------------------
// server/server_entity_system.cpp
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
// Copyright 2013-2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "server_entity_system.hpp"

#include <hexa/ip_address.hpp>

namespace hexa
{

server_entity_system::server_entity_system()
{
    auto check1(register_component<ip_address>("ipaddr"));
    auto check2(register_component<uint64_t>("player_uid"));

    if (!es::is_flat<ip_address>::value)
        throw std::runtime_error("ip_address object is not flat");

    if (check1 != c_ip_addr)
        throw std::runtime_error("cannot register component ipaddr");

    if (check2 != c_player_uid)
        throw std::runtime_error("cannot register component player_uid");
}

network_send_t network_send_behavior(es::storage::component_id component_id)
{
    switch (component_id) {
    case entity_system::c_position:
        return nearby_falloff;
    case entity_system::c_velocity:
        return nearby_falloff;
    case entity_system::c_force:
        return nearby_falloff;
    case entity_system::c_walk:
        return nearby_falloff;
    case entity_system::c_orientation:
        return nearby_falloff;
    case entity_system::c_boundingbox:
        return nearby;
    case entity_system::c_impact:
        return player_private;
    case entity_system::c_model:
        return nearby;
    case entity_system::c_name:
        return nearby;
    case entity_system::c_lookat:
        return nearby_falloff;
    case entity_system::c_lag_comp:
        return nearby_falloff;
    case entity_system::c_hotbar:
        return player_private;

    default:
        return server_private;
    }
}

} // namespace hexa
