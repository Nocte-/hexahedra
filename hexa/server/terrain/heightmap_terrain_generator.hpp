//---------------------------------------------------------------------------
/// \file   server/terrain/heightmap_terrain_generator.hpp
/// \brief  Fill the world with a single material based on a height map.
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

#include <memory>
#include "terrain_generator_i.hpp"

namespace hexa
{

/** Fill everything below a heightmap with a single material. */
class heightmap_terrain_generator : public terrain_generator_i
{
    struct impl;
    std::unique_ptr<impl> pimpl_;

public:
    /**
     *
     * @param w  The game world
     * @param conf  Uses the following parameters:
     *   - area (default 'heightmap'): the area to use as a height map
     *   - material (default 'stone', 1): the material to fill the world with
     */
    heightmap_terrain_generator(world& w,
                                const boost::property_tree::ptree& conf);

    virtual ~heightmap_terrain_generator();

    void generate(world_terraingen_access& data, const chunk_coordinates& pos,
                  chunk& cnk);

    chunk_height estimate_height(world_terraingen_access& data,
                                 map_coordinates xy, chunk_height prev) const;

    std::vector<std::string> area_generator() const override
    {
        return {"surface"};
    }

    bool generate(world_terraingen_access& proxy, const std::string& type,
                  map_coordinates pos, area_data& data) const override;
};

} // namespace hexa
