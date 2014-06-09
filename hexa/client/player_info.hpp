//---------------------------------------------------------------------------
/// \file   hexa/client/player_info.hpp
/// \brief  Persistent storage of player name, UID, and keys
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

#include <string>

namespace hexa
{

struct player_info
{
    std::string name;
    std::string uid;
    std::string public_key;
    std::string private_key;
    std::string password;
};

player_info get_player_info();

void write_player_info(const player_info& info);

void generate_new_key(player_info& info);

void generate_new_uid(player_info& info);

} // namespace hexa
