//---------------------------------------------------------------------------
/// \file   server/terrain_generator_i.hpp
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
// Copyright 2012-2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <set>
#include <boost/property_tree/ptree.hpp>
#include <hexa/basic_types.hpp>
#include <hexa/chunk.hpp>
#include <hexa/height_chunk.hpp>
#include <hexa/world_subsection.hpp>

namespace hexa {

class world;

/** Interface class for terrain generation modules
 * The server generates the game world on the fly.  This is done by a chain of
 * generation modules.  Usually, the first few create the general shape of the
 * land, and later modules add ores, strata, fauna, caves, and other details.
 */
class terrain_generator_i
{
public:
    terrain_generator_i (world& w, const boost::property_tree::ptree&)
        : w_ (w)
    { }

    virtual ~terrain_generator_i() {}

    /** Generate a chunk of world data.
     * \param xyz   The chunk's position
     * \param dest  The container to be filled */
    virtual void generate (chunk_coordinates xyz, chunk& dest) = 0;

    /** Generate multiple chunks of world data.
     * \param xyz   The active chunk's position
     * \param dest  The container to be filled */
    virtual void generate (chunk_coordinates xyz,
                           world_subsection<chunk_ptr>& dest) { }

    /** Estimate the height of the terrain at a given map position.
     * \param xy  The chunk column
     * \param prev The height as determined by the previous terrain
     *             generators.  This will be \a undefined_height for the
     *             first generator.
     * \return All chunks with a z ordinate of this value or more are
     *          guarateed to be only air. */
    virtual chunk_height estimate_height (map_coordinates xy, chunk_height prev) const
        { return prev; }

    /** Return a list of relative chunk positions that this generator will
     ** use to place features.
     * Some generators need to write outside the current chunk (the one
     * passed to generate() ) -- for example the leaves of a tree might
     * spill into three neighboring chunks if the trunk is placed in one
     * of the corners.  Such generators should overload this function to
     * return a list of all the chunks it needs relative to the current one.
     * */
    virtual std::set<world_vector> span() const
        { return { world_vector(0,0,0) }; }

protected:
    world&  w_;
};

} // namespace hexa
