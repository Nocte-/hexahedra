//---------------------------------------------------------------------------
// client/gui/context.cpp
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
// Copyright (C) 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "context.hpp"

#include "canvas.hpp"
#include "theme.hpp"

namespace hexa
{
namespace gui
{

context::context(canvas& cnv, theme& thm)
    : cnv_(cnv)
    , thm_(thm)
{
}

void context::draw()
{
    for (auto& w : widgets_)
        w->draw(cnv_);
}

void context::process_event(const event& ev)
{
    for (auto& w : widgets_)
        w->process_event(ev);
}
}
} // namespace hexa::gui
