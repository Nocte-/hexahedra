//---------------------------------------------------------------------------
/// \file  hexa/storage_i.hpp
/// \brief Interface for persistent storage modules
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
// Copyright 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include "area_data.hpp"
#include "chunk.hpp"
#include "compression.hpp"
#include "lightmap.hpp"
#include "read_write_lockable.hpp"
#include "surface.hpp"

namespace hexa {

/** Interface for persistent storage modules. */
class storage_i : public readers_writer_lock
{
public:
    virtual ~storage_i() {}
    virtual void cleanup() {}

    virtual void store (map_coordinates   xy,  uint16_t index, area_ptr data) = 0;
    virtual void store (chunk_coordinates xyz, chunk_ptr data) = 0;
    virtual void store (chunk_coordinates xyz, lightmap_ptr data) = 0;
    virtual void store (chunk_coordinates xyz, surface_ptr data) = 0;
    virtual void store (map_coordinates   xy,  chunk_height height) = 0;

    virtual bool is_area_data_available       (map_coordinates   xy, uint16_t index) = 0;
    virtual bool is_chunk_available           (chunk_coordinates xyz) = 0;
    virtual bool is_lightmap_available        (chunk_coordinates xyz) = 0;
    virtual bool is_surface_available         (chunk_coordinates xyz) = 0;
    virtual bool is_coarse_height_available   (map_coordinates   xy) = 0;

    virtual area_ptr        get_area_data     (map_coordinates   xy, uint16_t index) = 0;
    virtual chunk_ptr       get_chunk         (chunk_coordinates xyz) = 0;
    virtual lightmap_ptr    get_lightmap      (chunk_coordinates xyz) = 0;
    virtual surface_ptr     get_surface       (chunk_coordinates xyz) = 0;
    virtual chunk_height    get_coarse_height (map_coordinates   xy) = 0;

    virtual compressed_data
    get_compressed_lightmap (chunk_coordinates xyz) = 0;

    virtual compressed_data
    get_compressed_surface (chunk_coordinates xyz) = 0;
};

//---------------------------------------------------------------------------

/** Get a single block from a source.
 *  This function incurs some overhead.  If you need to access multiple
 *  blocks in the same chunk, use the chunk's indexing operators instead. */
inline block
get_block(storage_i& from, world_coordinates pos, block default_block = block())
{
    chunk_ptr find (from.get_chunk(pos / chunk_size));
    if (find == nullptr)
        return default_block;

    return (*find)[pos % chunk_size];
}

/** Check if all six of a chunk's cardinal neighbors are available. */
/*
inline bool
are_neighboring_chunks_available(storage_i& from, chunk_coordinates pos)
{
    return    from.is_chunk_available(pos + world_vector( 1, 0, 0))
           && from.is_chunk_available(pos + world_vector(-1, 0, 0))
           && from.is_chunk_available(pos + world_vector(0,  1, 0))
           && from.is_chunk_available(pos + world_vector(0, -1, 0))
           && from.is_chunk_available(pos + world_vector(0, 0,  1))
           && from.is_chunk_available(pos + world_vector(0, 0, -1));
}
*/

} // namespace hexa

