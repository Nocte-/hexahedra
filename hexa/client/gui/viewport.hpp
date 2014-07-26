//---------------------------------------------------------------------------
/// \file   client/gui/viewport.hpp
/// \brief  Viewport
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

#include "widget.hpp"

namespace hexa
{
namespace gui
{

class viewport : public widget
{
public:
    viewport(std::unique_ptr<widget> child);

    virtual void draw(canvas& cnv);

    virtual void process_event(const event& ev);

    virtual void on_mouse_enter();
    virtual void on_mouse_leave();

protected:
    std::unique_ptr<widget> child_;
};
}
} // namespace hexa::gui
