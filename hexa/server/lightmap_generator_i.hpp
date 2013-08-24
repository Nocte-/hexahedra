//---------------------------------------------------------------------------
/// \file   lightmap_generator_i.hpp
/// \brief  Interface for lightmap generators.
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

#include <boost/property_tree/ptree.hpp>
#include <hexa/chunk.hpp>
#include <hexa/lightmap.hpp>
#include <hexa/storage_i.hpp>
#include <hexa/surface.hpp>

namespace hexa {

/** Interface for lightmap generators. */
class lightmap_generator_i
{
public:
    lightmap_generator_i (storage_i& cache,
                          const boost::property_tree::ptree& conf)
        : cache_ (cache)
        , config_(conf)
    { }

    virtual ~lightmap_generator_i () {}

    /** Generate a lightmap for a chunk.
     * @param pos   The chunk's position
     * @param srf   The exposed faces in this chunk
     * @param map   The results will be placed in this chunk
     * @param phase Level of detail \sa phases
     * @return Reference to \a map */
    virtual lightmap& generate (const chunk_coordinates& pos,
                                const surface& srf,
                                lightmap& map,
                                unsigned int phase = 0) const = 0;

    /** The number of phases this generator needs.
     *  Light maps can be expensive to generate, but new terrain should
     *  also be pushed out to the players as fast as possible.  This is
     *  where phases come in.  Light maps always start at phase 0, which
     *  should be as fast as possible, accuracy be damned.  The server
     *  will call the generators again with higher phase numbers at a
     *  later time, depending on player distance, CPU load, etc.  This
     *  function should return the number of phases this generator supports. */
    virtual unsigned int phases() const { return 1; }

protected:
    storage_i&  cache_; /**< The game world. */
    boost::property_tree::ptree config_; /**< This module's configuration. */
};

} // namespace hexa

