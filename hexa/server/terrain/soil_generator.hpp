//---------------------------------------------------------------------------
/// \file   server/terrain/soil_generator.hpp
/// \brief  Cover the surface with different materials.
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
// Copyright 2013-2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <vector>
#include "terrain_generator_i.hpp"

namespace hexa
{

/** Replace the top number of blocks with a different material.
 *  This generator uses a "biome" map to look up the replacement materials.
 */
class soil_generator : public terrain_generator_i
{
public:
    /** The following configuration elements are used:
     * - 'surface_map': The area that holds the surface height map (default:
     * 'surface')
     * - 'distribution_map': The area with the biome classification. (default:
     * 'biome')
     * - 'original_material': The name of the material that will be replaced
     * (default: 'stone')
     * - 'replace': An array of an array of material names.  The value
     *   returned by 'distribution_map' will be used as the index.  The
     *   indexed array is then used as a column of different materials, the
     *   one at the first position will be the topmost block. */
    soil_generator(world& w, const boost::property_tree::ptree& conf);

    void generate(world_terraingen_access& data, const chunk_coordinates& pos,
                  chunk& cnk) override;

private:
    int surfacemap_;
    int biome_map_;
    block original_;
    std::vector<std::vector<block>> replace_;
};

} // namespace hexa
