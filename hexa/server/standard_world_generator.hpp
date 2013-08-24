//---------------------------------------------------------------------------
/// \file   server/standard_world_generator.hpp
/// \brief  Basic world generator based on Perlin noise.
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

#include <memory>
#include "terrain_generator_i.hpp"

namespace hexa {

/** Basic world generator.
 *  This generator is useful as a first step in the chain.  It uses
 *  several octaves of Perlin noise to generate a height map. */
class standard_world_generator : public terrain_generator_i
{
    struct impl;
    std::unique_ptr<impl> pimpl_;

public:
    standard_world_generator(world& w,
                             const boost::property_tree::ptree& conf);

    virtual ~standard_world_generator();

    void generate (chunk_coordinates pos, chunk& dest);

    chunk_height estimate_height (map_coordinates xy) const;
};

} // namespace hexa
