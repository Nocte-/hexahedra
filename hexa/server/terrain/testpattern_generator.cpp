//---------------------------------------------------------------------------
// server/terrain/testpattern_generator.cpp
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

#include "testpattern_generator.hpp"

#include <hexa/block_types.hpp>
#include <hexa/voxel_range.hpp>

#include "../random.hpp"

namespace hexa
{

void testpattern_generator::generate(world_terraingen_access&,
                                     const chunk_coordinates& pos, chunk& cnk)
{
    make_pattern(pos, cnk);
}

void testpattern_generator::make_pattern(const chunk_coordinates& pos,
                                         chunk& cnk) const
{
    if (pos.x < world_chunk_center.x) {
        for (auto p : every_block_in_chunk)
            cnk[p] = ((manhattan_length(p) % 2 == 0) & 0x0001);
    } else {
        auto hash(fnv_hash(pos));
        for (auto p : every_block_in_chunk) {
            cnk[p] = (hash & 0x0001);
            hash = prng(hash);
        }
    }
}

} // namespace hexa
