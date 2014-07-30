//---------------------------------------------------------------------------
/// \file   client/server_list.hpp
/// \brief  Retrieve and parse the list of game servers
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
// Copyright 2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace hexa
{

struct server_info
{
    std::string host;
    uint16_t port;
    std::string name;
    std::string desc;
    std::string icon;
    std::string screenshot;
    
    
};

std::vector<server_info> get_server_list(const std::string& json_uri);

} // namespace hexa
