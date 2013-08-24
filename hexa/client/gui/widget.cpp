//---------------------------------------------------------------------------
// client/gui/widget.cpp
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

#include "widget.hpp"

#include <hexa/rectangle_algorithms.hpp>

namespace hexa {
namespace gui {

widget::widget(context& owner)
    : context_(owner)
    , mouse_inside_flag_(false)
    , left_(0.0f), top_(0.0f), width_(0.0f), height_(0.0f)
{ }

void widget::process_event(const event& ev)
{
    switch (ev.type)
    {
        case event::mouse_move_abs:
            {
            bool inside (point_in_rectangle(ev.xy, bounding_rect()));
            if (inside != mouse_inside_flag_)
            {
                if (inside)
                    on_mouse_enter();
                else
                    on_mouse_leave();

                mouse_inside_flag_ = inside;
            }
            }
            break;

        default:
            ;
    }
}

rectangle<float> widget::bounding_rect() const
{
    float l (left().value()), t (top().value());
    return rectangle<float>(l, t, l + width().value(), t + height().value());
}

}} // namespace hexa::gui

