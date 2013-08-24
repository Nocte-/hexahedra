//---------------------------------------------------------------------------
/// \file   server/player.hpp
/// \brief
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

#include <string>
#include <enet/enet.h>
#include <es/entity.hpp>
#include <hexa/basic_types.hpp>
#include <hexa/player_base.hpp>

namespace hexa {

class player : public player_base
{
public:
    player();
    ~player();

public:
    ENetPeer*           conn;
    std::string         name;
    es::entity          entity;

    uint8_t             last_move_speed;
    uint8_t             last_move_dir;
    uint8_t             last_button_mask;
    uint8_t             selected_hotbar;
};

} // namespace hexa
