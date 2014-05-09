//---------------------------------------------------------------------------
/// \file   server/terrain/testpattern_generator.hpp
/// \brief  Generate fixed patterns for debugging and unit testing.
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
// Copyright 2012-2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include "terrain_generator_i.hpp"

namespace hexa {

/** This generator will create a test pattern.
 *  On the left half of the world (x < world_center.x), it generates
 *  a 3D checkerboard pattern.  The other half consists of pseudorandom
 *  noise. */
class testpattern_generator : public terrain_generator_i
{
public:
    testpattern_generator (world& w, const boost::property_tree::ptree& conf)
        : terrain_generator_i (w)
    { }

    void generate (world_terraingen_access& data,
                   const chunk_coordinates& pos,
                   chunk& cnk) override;

    void make_pattern (const chunk_coordinates& pos,
                       chunk& cnk) const;

    chunk_height
    estimate_height (world_terraingen_access&,
                     map_coordinates,
                     chunk_height) const override
    {
        return chunk_world_limit.z;
    }
};

} // namespace hexa

