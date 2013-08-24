//---------------------------------------------------------------------------
// server/gridworld_generator.cpp
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

#include "gridworld_generator.hpp"

#include <algorithm>
#include <hexa/block_types.hpp>

namespace hexa {

void gridworld_generator::generate(chunk_coordinates pos, chunk& dest)
{
    std::fill(dest.begin(), dest.end(), type::air);
    int j (chunk_size - 1);
    for (int i(0); i < chunk_size; ++i)
    {
        dest(i,0,0) = 16;
        dest(0,i,0) = 16;
        dest(0,0,i) = 16;

        dest(i,j,0) = 32;
        dest(0,i,j) = 32;
        dest(j,0,i) = 32;

        dest(i,0,j) = 48;
        dest(j,i,0) = 48;
        dest(0,j,i) = 48;

        dest(i,j,j) = 64;
        dest(j,i,j) = 64;
        dest(j,j,i) = 64;
    }
}

} // namespace hexa

