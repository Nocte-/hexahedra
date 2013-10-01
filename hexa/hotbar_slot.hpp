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

#include <vector>
#include "basic_types.hpp"

namespace hexa {

/** A single slot in a player's hotbar.
 *  Like most parts of the user interface, the hotbar at the bottom center
 *  is controlled server-side.  It can be manipulated through Lua, and
 *  serialized over the network. */
struct hotbar_slot
{
    /** The looks and behavior of a slot depend on this type. */
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

    hotbar_slot (char type_ = empty, std::string name_ = "")
        : type (type_)
        , name (name_)
        , counter (0xffff)
		, progress_bar (0)
        , can_drag (false)
        , cooldown (0)
    { }

    template<class archive>
    archive& serialize(archive& ar)
    {
        return ar(type)(name)(tooltip)(badge)(counter)(progress_bar)
                 (wield)(can_drag)(cooldown);
    }

    /** The slot type. */
    char         type;
    /** The name of the resource (material, item, icon) */
    std::string  name;
    /** Optional: a tooltip. */
    std::string  tooltip;
    /** Optional: an icon that will be drawn in the corner. */
    std::string  badge;
    /** Optional: a counter. */
    uint16_t     counter;
    /** Optional: a progress bar. */
    char         progress_bar;
    /** Optional: overriding the standard 3-D model that will be wielded. */
    std::string  wield;
    /** Whether the player can drag the icon out of the hotbar. */
    bool         can_drag;
    /** Any non-zero value will disable this action. */
    uint8_t      cooldown;
};

using hotbar = std::vector<hotbar_slot>;

// No support for type aliases in Visual Studio :.(
/*
class hotbar : private std::vector<hotbar_slot>
{
    typedef std::vector<hotbar_slot> base;

public:
    typedef base::iterator          iterator;
    typedef base::const_iterator    const_iterator;
    typedef base::value_type        value_type;
    typedef base::size_type         size_type;

    using base::operator[];
    using base::at;
    using base::begin;
    using base::end;
    using base::cbegin;
    using base::cend;
    using base::size;
    using base::empty;
    using base::resize;
    using base::clear;

public:
    hotbar() { }
    hotbar(size_type i) : base(i) { }
    hotbar(const hotbar& copy) : base(copy) { }
    hotbar(hotbar&& m) : base(std::move(m)) { }

    hotbar& operator= (hotbar&& m) { base::operator=(std::move(m)); return *this; }
    hotbar& operator= (const hotbar& c) { base::operator=(c); return *this; }

    template<class archive>
    archive& serialize(archive& ar)
    {
        std::vector<hotbar_slot>* p (this);
        return ar(*p);
    }
};
*/

} // namespace hexa

