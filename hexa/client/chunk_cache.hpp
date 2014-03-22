//---------------------------------------------------------------------------
/// \file   client/chunk_cache.hpp
/// \brief  Client memory cache for persistent_storage_i
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
// Copyright 2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <mutex>

#include <hexa/basic_types.hpp>
#include <hexa/compression.hpp>
#include <hexa/lightmap.hpp>
#include <hexa/lru_cache.hpp>
#include <hexa/surface.hpp>

namespace hexa {

class persistent_storage_i;

/** Simple memory cache for the client.
 *  This object keeps uncompressed versions of recently used chunk data in
 *  memory for fast access. */
class chunk_cache
{
public:
    chunk_cache (persistent_storage_i& store, size_t limit = 8192);

    void                cleanup();

    bool                is_coarse_height_available (const map_coordinates& pos) const;
    chunk_height        get_coarse_height (const map_coordinates& pos);
    void                store_coarse_height (const map_coordinates& pos,
                                             chunk_height value);

    bool                is_surface_available (const chunk_coordinates& pos) const;
    const surface_data& get_surface (const chunk_coordinates& pos);
    void                store_surface (const chunk_coordinates& pos,
                                       const compressed_data& data);

    bool                is_lightmap_available (const chunk_coordinates& pos) const;
    const light_data&   get_lightmap (const chunk_coordinates& pos);
    void                store_lightmap (const chunk_coordinates& pos,
                                        const compressed_data& data);

    inline bool         is_air (const chunk_coordinates& pos)
    {
        if (!is_coarse_height_available(pos))
            return false;

        return is_air_chunk(pos, get_coarse_height(pos));
    }

private:
    persistent_storage_i& store_;
    size_t                limit_;

    lru_cache<map_coordinates,   chunk_height>  heights_;
    mutable std::mutex                          heights_mutex_;

    lru_cache<chunk_coordinates, surface_data>  surfaces_;
    mutable std::mutex                          surfaces_mutex_;

    lru_cache<chunk_coordinates, light_data>    lightmaps_;
    mutable std::mutex                          lightmaps_mutex_;
};

} // namespace hexa
