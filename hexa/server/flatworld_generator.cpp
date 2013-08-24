//---------------------------------------------------------------------------
// server/flatworld_generator.cpp
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

#include "flatworld_generator.hpp"

#include <algorithm>
#include <hexa/block_types.hpp>

namespace hexa {

void flatworld_generator::generate(chunk_coordinates pos, chunk& dest)
{
/*
    if (pos == world_chunk_center)
    {
        std::fill(dest.begin(), dest.end(), type::air);
        dest(0,0,0) = type::grass;
        dest(0,1,0) = type::grass;
        dest(1,0,0) = type::grass;
        dest(2,0,0) = type::grass;
        dest(0,0,1) = type::grass;
        dest(15, 0, 0) = type::grass;
        dest(14, 0, 0) = type::grass;
        dest(0, 15, 0) = type::grass;
        dest(0, 0, 15) = type::grass;
    }
    else
 */
    if (!is_air_chunk(pos, level_))
        std::fill(dest.begin(), dest.end(), block_type_);
}

} // namespace hexa

