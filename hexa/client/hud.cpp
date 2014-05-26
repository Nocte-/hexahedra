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
    , show_debug_info (false)
    , local_height (undefined_height)
    , console_timeout_(12.0f)
    , max_msgs_ (20)
    , show_input_ (false)
    , input_cursor_ (0)
{}

void hud::time_tick(float seconds)
{
    for (auto& m : console_)
    {
        m.time -= seconds;
        if (m.time < 0.0f)
            m.time = 0.0f;
    }
}

void hud::console_message_timeout(float seconds)
{
    console_timeout_ = seconds;
}

void hud::console_message(const std::string& msg)
{
    console_.push_back({ msg, console_timeout_ });
    if (console_.size() > max_msgs_)
        console_.pop_front();

}

std::vector<std::string>
hud::console_messages() const
{
    std::vector<std::string> result;
    for (auto& m : console_)
    {
        if (m.time > 0)
            result.push_back(m.msg);
    }
    return result;
}

std::vector<std::string>
hud::old_console_messages() const
{
    std::vector<std::string> result;
    for (auto& m : console_)
        result.push_back(m.msg);

    return result;
}

void hud::set_cursor(unsigned int pos)
{
    input_cursor_ = pos;
}

void hud::set_input(const std::u32string &msg)
{
    input_ = msg;
}

} // namespace hexa
