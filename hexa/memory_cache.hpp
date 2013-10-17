//---------------------------------------------------------------------------
/// \file   memory_cache.hpp
/// \brief  Memory cache for game data
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

#include <unordered_set>
#include <boost/thread/mutex.hpp>
#include "lru_cache.hpp"
#include "persistent_storage_i.hpp"
#include "storage_i.hpp"

namespace hexa {

/** Memory cache for game data.
 *  This cache sits on top of a persistent storage.  It keeps the most recently
 *  used data uncompressed in memory, and makes sure storage and retrieval is
 *  done in batches if possible. */
class memory_cache : public storage_i, boost::noncopyable
{
    typedef std::unordered_set<chunk_coordinates>   dirty_flags;

    /** Area data cache.
     *  Note that it is indexed by chunk_coordinates, not map_coordinates.
     *  This is a bit of a hack; there can be different types of area data
     *  for a given position.  The z-ordinate is abused to store the type
     *  index. */
    lru_cache<chunk_coordinates, area_ptr>      areas_;
    dirty_flags  areas_dirty_;
    boost::mutex areas_mutex_;

    lru_cache<chunk_coordinates, chunk_ptr>     chunks_;
    dirty_flags  chunks_dirty_;
    boost::mutex chunks_mutex_;

    lru_cache<chunk_coordinates, lightmap_ptr>  lightmaps_;
    dirty_flags  lightmaps_dirty_;
    boost::mutex lightmaps_mutex_;

    lru_cache<chunk_coordinates, surface_ptr>   surfaces_;
    dirty_flags  surfaces_dirty_;
    boost::mutex surfaces_mutex_;

    lru_cache<map_coordinates,   chunk_height>  heights_;
    boost::mutex heights_mutex_;

    persistent_storage_i& next_;

public:
    memory_cache(persistent_storage_i& next, size_t sizelim = 25000)
        : next_ (next), size_limit_ (sizelim) { }

    void cleanup();

    ~memory_cache();

    void store (map_coordinates   xy,  uint16_t index, area_ptr data);
    void store (chunk_coordinates xyz, chunk_ptr data);
    void store (chunk_coordinates xyz, lightmap_ptr data);
    void store (chunk_coordinates xyz, surface_ptr data);
    void store (map_coordinates   xy,  chunk_height height);
    void store (const locked_subsection& region);

    bool is_area_data_available       (map_coordinates   xy, uint16_t index);
    bool is_chunk_available           (chunk_coordinates xyz);
    bool is_lightmap_available        (chunk_coordinates xyz);
    bool is_surface_available         (chunk_coordinates xyz);
    bool is_coarse_height_available   (map_coordinates   xy);

    area_ptr        get_area_data     (map_coordinates   xy, uint16_t index);
    chunk_ptr       get_chunk         (chunk_coordinates xyz);
    lightmap_ptr    get_lightmap      (chunk_coordinates xyz);
    surface_ptr     get_surface       (chunk_coordinates xyz);
    chunk_height    get_coarse_height (map_coordinates   xy);

    compressed_data
    get_compressed_lightmap (chunk_coordinates xyz);

    compressed_data
    get_compressed_surface (chunk_coordinates xyz);

private:
    size_t      size_limit_;
};

} // namespace hexa

