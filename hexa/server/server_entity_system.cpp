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

namespace hexa {

server_entity_system::server_entity_system()
{
    auto check1 (register_component<ip_address>("ipaddr"));
    auto check2 (register_component<std::vector<char>>("uuid"));

    if (!es::is_flat<ip_address>::value)
        throw std::runtime_error("ip_address object is not flat");

    if (check1 != c_ip_addr)
        throw std::runtime_error("cannot register component ipaddr");

    if (check2 != c_uuid)
        throw std::runtime_error("cannot register component uuid");
}

} // namespace hexa
