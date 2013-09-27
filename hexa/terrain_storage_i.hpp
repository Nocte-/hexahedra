//---------------------------------------------------------------------------
/// \file   terrain_storage_i.hpp
/// \brief  Interface for modules that can store terrain data
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

#include <boost/signals2.hpp>
#include <boost/thread/mutex.hpp>
#include "terrain_source_i.hpp"
#include "compression.hpp"
#include "surface.hpp"

namespace hexa {

/// Interface for modules that can store terrain data.
class terrain_storage_i : public terrain_source_i
{
public:
    virtual ~terrain_storage_i() {}

    /// Store some area data.
    virtual void store (const map_coordinates& xy, uint16_t index, area_ptr data) = 0;

    /// Store a chunk of terrain data.
    virtual void store (const chunk_coordinates& xyz, chunk_ptr data) = 0;

    /// Store a lightmap.
    virtual void store (const chunk_coordinates& xyz, lightmap_ptr data) = 0;

    /// Store the height at chunk resolution.
    virtual void store (const map_coordinates& xy, chunk_height data) = 0;

    /// Store the chunk surface
    virtual void store (const chunk_coordinates& xyz, surface_ptr data) = 0;

    virtual bool is_surface_available (const chunk_coordinates& xyz) = 0;

    /// Retrieve a chunk of terrain data in compressed form.
    // @param xyz   The chunk's coordinates
    // @return The compressed chunk data
    virtual std::vector<char>
    get_compressed_chunk (const chunk_coordinates& xyz) = 0;

    /// Retrieve a chunk of lightmap data in compressed form.
    // @param xyz   The chunk's coordinates
    // @return The compressed lightmap data
    virtual std::vector<char>
    get_compressed_lightmap (const chunk_coordinates& xyz) = 0;

    /// Retrieve a chunk of surface data in compressed form.
    // @param xyz   The chunk's coordinates
    // @return The compressed surface data
    virtual std::pair<compressed_data, compressed_data>
    get_compressed_surface (const chunk_coordinates& xyz) = 0;

    surface_ptr get_surface (const chunk_coordinates& xyz)
    {
        auto p (get_compressed_surface(xyz));
        surface_ptr result (new surface_data);
        result->opaque = decompress<surface>(p.first);
        result->transparent = decompress<surface>(p.second);
        return result;
    }

public:
    /// This function will be called at regular intervals.
    //  If the storage needs to do some maintenance work, such as writing
    //  buffers to file, it should be implemented here.
    virtual void cleanup() { }

public:
    /// Signal for updated chunks.
    boost::signals2::signal<void(const chunk_coordinates&, chunk_ptr)>
        on_update_chunk;

    /// Signal for updated lightmaps.
    boost::signals2::signal<void(const chunk_coordinates&, lightmap_ptr)>
        on_update_lightmap;

    /// 
    boost::mutex    lock;
};

} // namespace hexa

