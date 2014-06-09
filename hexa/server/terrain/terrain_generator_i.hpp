//---------------------------------------------------------------------------
/// \file   server/terrain/terrain_generator_i.hpp
/// \brief  Interface of terrain generator modules.
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

#include <set>
#include <boost/property_tree/ptree.hpp>
#include <hexa/basic_types.hpp>
#include <hexa/chunk.hpp>

#include "../world_terraingen_access.hpp"
#include "../world_subsection.hpp"

namespace hexa
{

class world;

/** Interface class for terrain generation modules
 * The server generates the game world on the fly.  This is done by a chain of
 * generation modules.  Usually, the first few create the general shape of the
 * land, and later modules add ores, strata, flora, caves, and other details.
 */
class terrain_generator_i
{
public:
    terrain_generator_i(world& w)
        : w_(w)
    {
    }

    virtual ~terrain_generator_i() {}

    /** Generate part of the world.
     * @param data  Limited access to the world data
     * @param pos   The chunk coordinates
     * @param cnk   The chunk to fill */
    virtual void generate(world_terraingen_access& data,
                          const chunk_coordinates& pos, chunk& cnk) = 0;

    /** Estimate the height of the terrain at a given map position.
     * @param data  Limited access to the world data
     * @param xy  The chunk column
     * @param prev The height as estimated by the previous terrain
     *             generators.  This will be \a undefined_height for the
     *             first generator.  Generators can safely ignore this value.
     * @return All chunks with a z ordinate of this value or more are
     *         guarateed to be only air. */
    virtual chunk_height estimate_height(world_terraingen_access& data,
                                         map_coordinates xy,
                                         chunk_height prev) const
    {
        return undefined_height;
    }

    /** If this module can also generate area data, it should return the
     ** names of the areas in this function. */
    virtual std::vector<std::string> area_generator() const { return {}; }

    virtual bool generate(world_terraingen_access& data,
                          const std::string& type, map_coordinates pos,
                          area_data& area) const
    {
        return false;
    }

protected:
    world& w_;
};

} // namespace hexa
