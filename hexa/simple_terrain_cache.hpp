//---------------------------------------------------------------------------
/// \file   simple_terrain_cache.hpp
/// \brief  Memory cache for terrain data.
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

#include "lru_cache.hpp"
#include "terrain_storage_i.hpp"

namespace hexa {

/** Memory cache for terrain data. */
class simple_terrain_cache : public terrain_storage_i
{
    /** Area data cache.
     *  Note that it is indexed by chunk_coordinates, not map_coordinates.
     *  This is a bit of a hack; there can be different types of area data
     *  for a given position.  The z-ordinate is abused to store the type
     *  index. */
    lru_cache<chunk_coordinates, area_ptr>      areadata_;

    lru_cache<chunk_coordinates, chunk_ptr>     chunks_;
    lru_cache<chunk_coordinates, lightmap_ptr>  lightmaps_;
    lru_cache<chunk_coordinates, surface_ptr>   surfaces_;
    lru_cache<map_coordinates,   chunk_height>  height_;

    terrain_storage_i& next_;

public:
    simple_terrain_cache(terrain_storage_i& next, size_t sizelim = 5120)
        : next_ (next), size_limit_ (sizelim) { }

    void cleanup();

    /** */
    virtual ~simple_terrain_cache();

    void store (const map_coordinates& xy, uint16_t index, area_ptr data);

    void store (const chunk_coordinates& xyz, chunk_ptr data);

    void store (const chunk_coordinates& xyz, lightmap_ptr data);

    void store (const chunk_coordinates& xyz, surface_ptr data);

    void store (const map_coordinates& xy, chunk_height height);


    bool is_area_data_available (const map_coordinates& xy, uint16_t index);

    bool is_chunk_available (const chunk_coordinates& xyz);

    bool is_lightmap_available (const chunk_coordinates& xyz);

    bool is_surface_available (const chunk_coordinates& xyz);

    bool is_coarse_height_available (const map_coordinates& xy);


    area_ptr        get_area_data (const map_coordinates& xy, uint16_t index);

    chunk_ptr       get_chunk    (const chunk_coordinates& xyz);

    lightmap_ptr    get_lightmap (const chunk_coordinates& xyz);

    surface_ptr     get_surface (const chunk_coordinates& xyz);

    chunk_height    get_coarse_height (const map_coordinates& xyz);


    std::vector<char>
    get_compressed_chunk (const chunk_coordinates& xyz);

    std::vector<char>
    get_compressed_lightmap (const chunk_coordinates& xyz);

    std::pair<compressed_data, compressed_data>
    get_compressed_surface (const chunk_coordinates& xyz);

private:
    size_t size_limit_;
};

} // namespace hexa

