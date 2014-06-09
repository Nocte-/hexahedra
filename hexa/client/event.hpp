//---------------------------------------------------------------------------
/// \file   client/event.hpp
/// \brief  User interface event.
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
// Copyright 2012-2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include <array>
#include <vector>

#include <hexa/vector2.hpp>
#include "keycodes.hpp"

namespace hexa
{

class game;

/** A single event, like a keyboard button press, or a mouse move. */
struct event
{
    typedef enum {
        key_up,
        key_down,
        key_text,
        mouse_move_rel,
        mouse_move_abs,
        mouse_button_up,
        mouse_button_down,
        mouse_wheel,
        joy_move,
        joy_button_up,
        joy_button_down,
        window_close
    } ev_type;

    /** Joystick axis identifier and position. */
    struct axis_info
    {
        /** Axis number. */
        uint8_t id;
        /** Position along the axis, range [-1 .. 1] */
        float position;
    };

    ev_type type;

    union
    {
        key keycode;
        uint32_t code;
        axis_info axis;
        int delta;
    };
    vector2<float> xy;

    event(ev_type t)
        : type(t)
    {
    }
    event(ev_type t, uint32_t c)
        : type(t)
        , code(c)
    {
    }
    event(ev_type t, const vector2<float>& c)
        : type(t)
        , xy(c)
    {
    }
    event(ev_type t, const axis_info& c)
        : type(t)
        , axis(c)
    {
    }
    event(ev_type t, int c)
        : type(t)
        , delta(c)
    {
    }
};

} // namespace hexa
