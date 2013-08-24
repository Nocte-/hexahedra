//---------------------------------------------------------------------------
/// \file   client/sprite_sheet.hpp
/// \brief  Convenience class for treating a texture as a sprite sheet.
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
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <hexa/basic_types.hpp>
#include <hexa/rectangle.hpp>
#include "texture.hpp"

namespace hexa {

class sprite_sheet
{
public:
    sprite_sheet (texture tex, uint16_t width, uint16_t height);

    void bind (vector2<uint16_t> index) const;

    void bind (rectangle<uint16_t> multi) const;

    void bind (uint32_t index) const
        { bind(vector2<uint16_t>(index % width, index / width); }

private:
    texture  tex_;
    uint16_t width_;
    uint16_t height_;
};

} // namespace hexa

