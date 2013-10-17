//---------------------------------------------------------------------------
/// \file   hexa/server/world.hpp
/// \brief  The game world.
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

#include <memory>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <es/storage.hpp>

#include <hexa/basic_types.hpp>
#include <hexa/chunk.hpp>
#include <hexa/concurrent_queue.hpp>
#include <hexa/height_chunk.hpp>
#include <hexa/locked_subsection.hpp>
#include <hexa/lightmap.hpp>
#include <hexa/storage_i.hpp>
#include <hexa/world_subsection.hpp>
#include <hexa/voxel_range.hpp>

#include "area_generator_i.hpp"
#include "terrain_generator_i.hpp"
#include "lightmap_generator_i.hpp"

namespace hexa {

class world : public storage_i
{
public:
    typedef enum
    {
        normal_chunk, air_chunk, unknown_chunk
    }
    chunk_type;

    struct request
    {
        enum type_t
        {
            chunk, surface, lightmap, surface_and_lightmap, quit
        };

        type_t                  type;
        chunk_coordinates       pos;
        std::function<void()>   answer;
    };

    concurrent_queue<request> requests;

protected:


public:
    world(storage_i& storage);
    virtual ~world();

    void        add_area_generator(std::unique_ptr<area_generator_i>&& gen);
    void        add_terrain_generator(std::unique_ptr<terrain_generator_i>&& gen);
    void        add_lightmap_generator(std::unique_ptr<lightmap_generator_i>&& gen);

    area_ptr    get_area_data(map_coordinates pos, uint16_t index);
    bool        is_area_data_available(map_coordinates pos, uint16_t index);
    void        store(map_coordinates pos, uint16_t index, area_ptr data);

    /** Retrieve a fully generated chunk, or a nullptr for an air chunk. */
    chunk_ptr   get_chunk(chunk_coordinates pos);
    bool        is_chunk_available(chunk_coordinates pos);
    void        store(chunk_coordinates pos, chunk_ptr data);

    lightmap_ptr    get_lightmap(chunk_coordinates pos);
    bool            is_lightmap_available(chunk_coordinates pos);
    void            store(chunk_coordinates pos, lightmap_ptr data);

    surface_ptr     get_surface(chunk_coordinates pos);
    bool            is_surface_available(chunk_coordinates pos);
    void            store(chunk_coordinates pos, surface_ptr data);

    chunk_height    get_coarse_height(map_coordinates pos);
    bool            is_coarse_height_available(map_coordinates pos);
    void            store(map_coordinates pos, chunk_height data);

    void            store(const locked_subsection& region) { }

    compressed_data
    get_compressed_lightmap (chunk_coordinates xyz);

    compressed_data
    get_compressed_surface (chunk_coordinates xyz);

    chunk_type  get_type(chunk_coordinates xyz) const;

    int         find_area_generator(const std::string& name) const;

    bool        is_air (chunk_coordinates xyz);

    std::tuple<world_coordinates, world_coordinates>
                raycast(const wfpos& origin, const yaw_pitch& direction,
                        float distance);

    void        change_block(world_coordinates pos, uint16_t material);
    void        change_block(world_coordinates pos, const std::string& material);
    block       get_block(world_coordinates pos);

    void        cleanup();

    chunk_ptr   get_raw_chunk(chunk_coordinates pos);



    std::unordered_set<chunk_coordinates> changeset;
    boost::mutex                          changeset_lock;
    std::unordered_set<map_coordinates>   height_changeset;
    boost::mutex                          height_changeset_lock;

protected:
    locked_subsection  lock_chunks (const std::set<chunk_coordinates>& r);

    block get_block_nolocking(world_coordinates pos);
    void  refine_lightmap (chunk_coordinates pos, int phase);

    /** Get an existing chunk, or if it doesn't exist yet, create an
     ** empty one.
     *  If needed, the coarse height map is also adjusted. */
    chunk_ptr       get_or_create_chunk(chunk_coordinates pos);
    lightmap_ptr    get_or_create_lightmap(chunk_coordinates pos);
    surface_ptr     get_or_create_surface(chunk_coordinates pos);

    /** Get an existing chunk, or if it doesn't exist yet, create one.
     *  If needed, the coarse height map is also adjusted.  The chunk is
     *  created in such a way that the terrain generators up to \a phase
     *  have been invoked.  If phase is zero, this is equivalent to
     *  get_or_create_chunk. */
    //chunk_ptr       get_or_generate_chunk(chunk_coordinates pos, int phase, bool adjust_height = true);

    /** Regenerate surface and lightmap data. */
    void  update (chunk_coordinates pos);

    void worker(int id);

    void set_coarse_height(chunk_coordinates pos, chunk_height h);

    void generate_terrain (chunk_coordinates pos, const chunk_ptr& chunk);
    void generate_terrain (chunk_coordinates pos, const chunk_ptr& chunk, int phase);

    unsigned int find_generator(const terrain_generator_i& requester) const;

protected:
    //es::storage  entities_;
    //boost::mutex entities_mutex_;

private:
    std::vector<std::unique_ptr<area_generator_i>>      areagen_;
    std::vector<std::unique_ptr<terrain_generator_i>>   terraingen_;
    std::vector<std::unique_ptr<lightmap_generator_i>>  lightgen_;

    storage_i& storage_;

    int heightmap_;
    std::map<world_vector, unsigned int> phases_;

    std::vector<boost::thread>  workers_;
    boost::mutex                generator_lock_;

    boost::mutex check_lock_;
    std::unordered_set<chunk_coordinates> check_;
};

} // namespace hexa

