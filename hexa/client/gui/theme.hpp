//---------------------------------------------------------------------------
/// \file   client/gui/theme.hpp
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
// Copyright 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

namespace hexa
{
namespace gui
{

/**  */
class theme
{
public:
    theme() {}
    virtual ~theme() {}

    virtual widget::ptr text_label(const std::string& text) const = 0;

    virtual widget::ptr button(widget::ptr&& contents,
                               std::function<void()> on_activate) const = 0;

    virtual float default_spacing() const { return 5.0f; }
};
}
} // namespace hexa::gui
