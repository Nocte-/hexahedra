//---------------------------------------------------------------------------
/// \file   client/gui/types.hpp
/// \brief  Type definitions used in the GUI
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

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "rectangle.hpp"

namespace gui
{

typedef glm::ivec2 pos;
typedef glm::dvec4 color;
typedef rectangle<pos> rect;

} // namespace gui
