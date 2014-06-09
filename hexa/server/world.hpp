//---------------------------------------------------------------------------
/// \file   hexa/server/world.hpp
/// \brief  The game world
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

#include <memory>
#include <mutex>
#include <set>
#include <unordered_map>
#include <vector>

#include <boost/signals2.hpp>

#include <hexa/basic_types.hpp>
#include <hexa/chunk.hpp>
#include <hexa/compression.hpp>
#include <hexa/container_uptr.hpp>
#include <hexa/lightmap.hpp>
#include <hexa/lru_cache.hpp>
#include <hexa/persistent_storage_i.hpp>
#include <hexa/read_write_lockable.hpp>
#include <hexa/surface.hpp>

#include "area/area_generator_i.hpp"
#include "lightmap/lightmap_generator_i.hpp"
#include "terrain/terrain_generator_i.hpp"

#include "world_read.hpp"
#include "world_write.hpp"

namespace hexa
{

class world_lightmap_access;
class world_terraingen_access;

/** The game world.
 *  This class takes care of several things:
 *  - Keeping chunk data (and the associated surfaces, light maps, and
 *    compressed data) in a memory cache.
 *  - Writing the cached data to disk on changes.
 *  - Calling the terrain generators when new chunks are accessed.
 *  - Providing mutexed read and write access to the rest of the application.
 */
class world
{
    friend class world_read;
    friend class world_write;
    friend class world_terraingen_access;
    friend class world_lightmap_access;

public:
    boost::signals2::signal<void(chunk_coordinates)> on_update_coarse_height;
    boost::signals2::signal<void(chunk_coordinates)> on_update_surface;

public:
    world(persistent_storage_i& storage);

    world(const world&) = delete;
    // world(world&&) = default;
    // world& operator=(world&&) = default;

    /** Get read access to a given position. */
    world_read acquire_read_access();

    /** Get write access to a given chunk. */
    world_write acquire_write_access(const chunk_coordinates& pos);

    /** Terrain generation seed. */
    uint32_t seed() const { return seed_; }

public:
    /** Add an area generator. */
    void add_area_generator(std::unique_ptr<area_generator_i>&& gen);

    size_t nr_of_registered_area_generators() const { return areagen_.size(); }

    /** Look up an area generator by name.
     * @param name  The name to look for
     * @return The index of the area generator, or -1 if it doesn't exist. */
    int find_area_generator(const std::string& name) const;

    /** Add a terrain generator.
     *  The order in which terrain generators are added is important.  For
     *  example, if you add a cave system after the trees have already been
     *  planted, it will dig the caves through the trees as well. */
    void add_terrain_generator(std::unique_ptr<terrain_generator_i>&& gen);

    /** Add a lightmap generator. */
    void add_lightmap_generator(std::unique_ptr<lightmap_generator_i>&& gen);

    /** Flush data from memory to disk. */
    void cleanup();

protected: // Only available through world_read and world_write
           /** world_read and world_write use this to synchronize */
    // readers_writer_lock  lock;
    std::mutex single_lock;

    /** Returns a read-only chunk. */
    const chunk& get_chunk(chunk_coordinates pos);

    /** Returns a chunk that can be written to.
     *  After the write operation is done, the destructor of the
     *  world_write object will call commit_write(). */
    chunk& get_chunk_writable(chunk_coordinates pos);

    /** Commit the changes to a chunk.
     *  This will make sure the database, surfaces, light maps, and
     *  compressed data will be updated accordingly. */
    void commit_write(chunk_coordinates pos);

    const area_data& get_area_data(map_coordinates pos, uint16_t index);

    area_data& get_area_data_writable(map_coordinates pos, uint16_t index);

    const surface_data& get_surface(chunk_coordinates pos);

    const light_data& get_lightmap(chunk_coordinates pos);

    chunk_height get_coarse_height(map_coordinates pos);

    compressed_data get_compressed_chunk(chunk_coordinates pos);

    compressed_data get_compressed_surface(chunk_coordinates pos);

    compressed_data get_compressed_lightmap(chunk_coordinates pos);

    bool is_area_available(map_coordinates pos, uint16_t idx) const;

    /** Check if a chunk is available for use by the rest of the engine.
     *  This means that the chunk is completely finished, and it is
     *  either loaded in memory or can be fetched from file.  If this
     *  returns false, a subsequent call to get_compressed_chunk() will
     *  trigger terrain generation. */
    bool is_chunk_available(chunk_coordinates pos) const;

    /** Check if a surface is available for use.
     *  This may return false even though the chunk is finished.  If this
     *  is the case, a subsequent call to get_compressed_surface() might
     *  trigger terrain generation for pos, and its six neighbors. */
    bool is_surface_available(chunk_coordinates pos) const;

    bool is_lightmap_available(chunk_coordinates pos) const;

private:
    /** Generate the terrain of a given chunk. */
    chunk generate_chunk(chunk_coordinates pos);

    /** Generate the lightmap of a given chunk. */
    light_data generate_lightmap(chunk_coordinates pos, int level = 0);

    chunk_height generate_coarse_height(map_coordinates pos);

    chunk_height set_coarse_height(chunk_coordinates pos);

    void adjust_coarse_height(chunk_coordinates pos);

    /** Build a new surface at the given location. */
    surface_data build_surface(chunk_coordinates pos);

private:
    persistent_storage_i& storage_;

    vector_uptr<area_generator_i> areagen_;
    vector_uptr<terrain_generator_i> terraingen_;
    vector_uptr<lightmap_generator_i> lightgen_;

    template <typename t>
    using cache_map = lru_cache<chunk_coordinates, t>;

    cache_map<area_data> area_data_;
    cache_map<chunk> chunks_;
    cache_map<surface_data> surfaces_;
    cache_map<light_data> lightmaps_;

    lru_cache<map_coordinates, chunk_height> coarse_heights_;

    uint32_t seed_;
};

//--------------------------------------------------------------------------

/** Convenience function to read a single block. */
inline uint16_t get_block(world& w, world_coordinates pos)
{
    return w.acquire_read_access().get_block(pos);
}

/** Convenience function to read the coarse map height. */
inline chunk_height coarse_height(world& w, map_coordinates pos)
{
    return w.acquire_read_access().get_coarse_height(pos);
}

inline void prepare_for_player(world& w, chunk_coordinates pos)
{
    auto proxy(w.acquire_read_access());
    proxy.get_compressed_surface(pos);
    proxy.get_compressed_lightmap(pos);
}

/** Cast a ray through the game world and return the first hit. */
std::tuple<world_coordinates, world_coordinates>
raycast(world& w, const wfpos& origin, const yaw_pitch& direction,
        float distance);

} // namespace hexa
