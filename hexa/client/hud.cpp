//---------------------------------------------------------------------------
// hexa/client/hud.cpp
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

#include "hud.hpp"

namespace hexa {

hud::hud()
    : hotbar_needs_update (false)
    , active_slot (0)
    , console_timeout_(12.0f)
    , input_cursor_ (0)
{}

void hud::time_tick(float seconds)
{
    for (auto i (console_.begin()); i != console_.end(); )
    {
        i->time -= seconds;
        if (i->time <= 0.0f)
            i = console_.erase(i);
        else
            ++i;
    }
}

void hud::console_message_timeout(float seconds)
{
    console_timeout_ = seconds;
}

void hud::console_message(const std::string& msg)
{
    console_.push_back({ msg, console_timeout_ });
}

} // namespace hexa
