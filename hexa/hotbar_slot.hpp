//---------------------------------------------------------------------------
/// \file   hotbar_slot.hpp
/// \brief  A slot in a player's hotbar
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

#include <string>
#include <vector>
#include "basic_types.hpp"

#include <iostream>

namespace hexa
{

/** A single slot in a player's hotbar.
 *  Like most parts of the user interface, the hotbar at the bottom center
 *  is controlled server-side.  It can be manipulated through Lua, and
 *  serialized over the network. */
struct hotbar_slot
{
    /** The looks and behavior of a slot depend on this type. */
    // Note: should have been an enum class, but not supported in msvc.
    enum slot_type {
        /** An empty slot. */
        empty = 0,
        /** A block, usually drawn with the material textures. */
        material = 1,
        /** An item, usually one from the player's inventory. */
        item = 2,
        /** A stock icon, for actions. */
        icon = 3
    };

    hotbar_slot(char type_ = empty, std::string name_ = "")
        : type{type_}
        , name{name_}
        , counter{0xffff}
        , progress_bar{0}
        , can_drag{false}
        , cooldown{0}
    {
    }

    template <typename Archive>
    Archive& serialize(Archive& ar)
    {
        return ar(type)(name)(tooltip)(badge)(counter)(progress_bar)(wield)(
            can_drag)(cooldown);
    }

    /** The slot type. */
    char type;
    /** The name of the resource (material, item, icon) */
    std::string name;
    /** Optional: a tooltip. */
    std::string tooltip;
    /** Optional: an icon that will be drawn in the corner. */
    std::string badge;
    /** Optional: a counter. */
    uint16_t counter;
    /** Optional: a progress bar. */
    char progress_bar;
    /** Optional: overriding the standard 3-D model that will be wielded. */
    std::string wield;
    /** Whether the player can drag the icon out of the hotbar. */
    bool can_drag;
    /** Any non-zero value will disable this action. */
    uint8_t cooldown;
};

using hotbar = std::vector<hotbar_slot>;

} // namespace hexa

namespace std
{

inline ostream& operator<<(ostream& in, const hexa::hotbar_slot& x)
{
    return in << "hotbar slot";
}

inline string to_string(const hexa::hotbar_slot& x)
{
    return "hotbar slot";
}

inline string to_string(const hexa::hotbar& x)
{
    return "hotbar";
}

} // namespace std
