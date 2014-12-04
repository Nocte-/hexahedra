//---------------------------------------------------------------------------
/// \file   hexa/client/gui/font_i.hpp
/// \brief  Interface class for fonts
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
#pragma once

#include <string>

namespace gui
{

class font_i
{
public:
    virtual ~font_i() {}

    virtual float line_spacing(float point_size) const
    {
        return point_size * 2.0f;
    }

    virtual float ascend(float point_size) const { return point_size; }

    virtual float descend(float point_size) const { return point_size; }

    float height(float point_size) const
    {
        return ascend(point_size) + descend(point_size);
    }
};

} // namespace gui
