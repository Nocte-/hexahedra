//---------------------------------------------------------------------------
/// \file   terrain_source_i.hpp
/// \brief  Interface for classes that instance terrain data.
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
#include <boost/utility.hpp>
#include "area_data.hpp"
#include "basic_types.hpp"
#include "chunk.hpp"
#include "height_chunk.hpp"
#include "lightmap.hpp"

namespace hexa {

/// A terrain source creates new instances of chunks.
//  Examples of sources are the world database, the terrain generator,
//  the light calculations, and the network code at the client side.
class terrain_source_i : boost::noncopyable
{
public:
    /** */
    virtual ~terrain_source_i() {}

    /// Create or retrieve area data.
    // @param xy   The chunk's coordinates
    // @return The chunk, or nullptr if the chunk was not found 
    virtual area_ptr get_area_data (const map_coordinates& xy, uint16_t index)
        { return area_ptr(); }

    /// Create or retrieve a chunk of terrain data.
    // @param xyz   The chunk's coordinates
    // @return The chunk, or nullptr if the chunk was not found 
    virtual chunk_ptr get_chunk (const chunk_coordinates& xyz)
        { return chunk_ptr(); }

    /// Get the coarse height level.
    //  All chunks at this height, and above it, are guaranteed to consist
    //  of nothing but air. 
    virtual chunk_height get_coarse_height (const map_coordinates& xy)
        { return undefined_height; }

    /// Create or retrieve a lightmap.
    // @param xyz   The lightmap's coordinates
    // @return The lightmap, or nullptr if the lightmap was not found
    virtual lightmap_ptr get_lightmap (const chunk_coordinates& xyz)
        { return lightmap_ptr(); }

    /// Check if there's area data available at a given position.
    // @param xy    The coordinates of the chunk to query
    // @param index The type of area data
    // @return true iff area data is available at the given position
    virtual bool
    is_area_data_available (const map_coordinates& xy, uint16_t index)
        { return false; }

    /// Check if there's a chunk available at a given position.
    // @param xyz   The coordinates of the chunk to query
    // @return true iff terrain data is available at the given position
    virtual bool
    is_chunk_available (const chunk_coordinates& xyz)
        { return false; }

    /// Check if there height is known at a given position.
    // @param xy   The chunk coordinates on the map
    // @return true iff the height is known at the given position
    virtual bool
    is_coarse_height_available (const map_coordinates& xy)
        { return false; }

    /// Check if there's a lightmap available at a given position.
    // @param xyz   The coordinates of the chunk to query
    // @return true iff a lightmap is available at the given position 
    virtual bool
    is_lightmap_available (const chunk_coordinates& xyz)
        { return false; }
};

//---------------------------------------------------------------------------

/// Get a single block from a terrain source.
//  This function incurs some overhead.  If you need to access multiple
//  blocks in the same chunk, use the chunk's indexing operators instead.
inline block
get_block(terrain_source_i& from, const world_coordinates& pos,
          block default_block = block())
{
    chunk_ptr find (from.get_chunk(pos / chunk_size));
    if (find == nullptr)
        return default_block;

    return (*find)[pos % chunk_size];
}

/// Check if all six of a chunk's cardinal neighbors are available.
inline bool
are_neighboring_chunks_available(terrain_source_i& from,
                                 const chunk_coordinates& pos)
{
    return    from.is_chunk_available(pos + world_vector( 1, 0, 0))
           && from.is_chunk_available(pos + world_vector(-1, 0, 0))
           && from.is_chunk_available(pos + world_vector(0,  1, 0))
           && from.is_chunk_available(pos + world_vector(0, -1, 0))
           && from.is_chunk_available(pos + world_vector(0, 0,  1))
           && from.is_chunk_available(pos + world_vector(0, 0, -1));
}

} // namespace hexa

