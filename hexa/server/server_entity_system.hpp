//---------------------------------------------------------------------------
/// \file   server/server_entities.hpp
/// \brief  Server-specific extensions to the entity system
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
// Copyright 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <hexa/entity_system.hpp>

namespace hexa {

class server_entity_system : public entity_system
{
public:
    enum server_components
    {
        c_ip_addr = c_last_component,
        c_last_server_component
    };

public:
    server_entity_system();
};

} // namespace hexa

