//---------------------------------------------------------------------------
// client/gui/mouse_event_handler.hpp
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
#include "mouse_event_handler.hpp"

namespace gui
{

void mouse_region::move_to(const pos& p)
{
    if (p == pos_)
        return;

    bool inside = point_on_rectangle(p, area_);
    if (inside) {
        pos_ = p;
        on_move(p);
        if (dragging_) {
            on_drag(p);
        }
    }
    if (inside != inside_) {
        inside_ = inside;
        if (inside_)
            on_enter();
        else
            on_leave();
    }
}

void mouse_region::button_down(int btn)
{
    dragging_ = true;
    if (inside_)
        on_button_down(pos_, btn);
}

void mouse_region::button_up(int btn)
{
    dragging_ = false;
    if (inside_)
        on_button_up(pos_, btn);
}

void mouse_region::wheel(int z)
{
    if (inside_)
        on_wheel(pos_, z);
}

void mouse_region::leave()
{
    if (inside_) {
        inside_ = false;
        on_leave();
    }
}

//---------------------------------------------------------------------------

void mouse_region_group::move_to(const pos& p)
{
    mouse_region::move_to(p);
    if (inside_) {
        for (auto& reg : items_)
            reg->move_to(p);
    }
}

void mouse_region_group::button_down(int btn)
{
    mouse_region::button_down(btn);
    if (inside_) {
        for (auto& reg : items_)
            reg->button_down(btn);
    }
}

void mouse_region_group::button_up(int btn)
{
    mouse_region::button_up(btn);
    if (inside_) {
        for (auto& reg : items_)
            reg->button_up(btn);
    }
}

void mouse_region_group::wheel(int z)
{
    mouse_region::wheel(z);
    if (inside_) {
        for (auto& reg : items_)
            reg->wheel(z);
    }
}

void mouse_region_group::leave()
{
    if (inside_) {
        for (auto& reg : items_)
            reg->leave();

        mouse_region::leave();
    }
}

//---------------------------------------------------------------------------

void mouse_region_switch::move_to(const pos& p)
{
    active().move_to(p);
}

void mouse_region_switch::button_down(int btn)
{
    active().button_down(btn);
}

void mouse_region_switch::button_up(int btn)
{
    active().button_up(btn);
}

void mouse_region_switch::wheel(int z)
{
    active().wheel(z);
}

void mouse_region_switch::leave()
{
    active().leave();
}

} // namespace gui
