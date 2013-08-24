//---------------------------------------------------------------------------
// server/world.cpp
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

#include "world.hpp"

#include <algorithm>

#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>

#include <es/component.hpp>
#include <es/entity.hpp>

#include <hexa/entity_system.hpp>
#include <hexa/entity_system_physics.hpp>
#include <hexa/geometric.hpp>
#include <hexa/memory_cache.hpp>
#include <hexa/surface.hpp>
#include <hexa/neighborhood.hpp>
#include <hexa/trace.hpp>
#include <hexa/voxel_algorithm.hpp>

#include <iostream>

using namespace boost::range;
using boost::format;

namespace hexa {

static chunk empty_chunk;

world::world (storage_i& storage)
    : storage_ (storage)
{
    for (int i (0); i < 1; ++i)
        workers_.emplace_back([=]{ worker(i); });
}

world::~world()
{
    // Message all worker threads to finish whatever they're doing, and
    // quit.
    for (unsigned int i (0); i < workers_.size(); ++i)
        requests.push({ request::quit, chunk_coordinates(), []{} });

    for (auto& wt : workers_)
        wt.join();
}

//---------------------------------------------------------------------------

world::exclusive_section::exclusive_section(std::vector<boost::unique_lock<boost::mutex>>&& locks)
    : locked_(std::move(locks))
{
}

world::exclusive_section::~exclusive_section()
{
    for (auto& l : locked_)
        l.unlock();
}

//---------------------------------------------------------------------------

void world::add_area_generator(std::unique_ptr<area_generator_i>&& gen)
{
    areagen_.emplace_back(std::move(gen));
}

void world::add_terrain_generator(std::unique_ptr<terrain_generator_i>&& gen)
{
    terraingen_.emplace_back(std::move(gen));
}

void world::add_lightmap_generator (std::unique_ptr<lightmap_generator_i>&& gen)
{
    lightgen_.emplace_back(std::move(gen));
}

//---------------------------------------------------------------------------

area_ptr
world::get_area_data (map_coordinates pos, uint16_t index)
{
    area_ptr result (storage_.get_area_data(pos, index));
    if (result)
        return result;

    // If we have a generator for it, create a brand new area data
    if (index < areagen_.size())
    {
        area_ptr new_result (new area_data);
        areagen_[index]->generate(pos, *new_result);
        store(pos, index, new_result);
        return new_result;
    }

    return nullptr;
}

void
world::store (map_coordinates pos, uint16_t index, area_ptr data)
{
    storage_.store(pos, index, data);
}

bool
world::is_area_data_available (map_coordinates pos, uint16_t index)
{
    auto lock (storage_.acquire_read_lock());
    return storage_.is_area_data_available(pos, index);
}

//---------------------------------------------------------------------------

chunk_ptr
world::get_chunk (chunk_coordinates pos)
{
    // Return nothing if it's an all-air chunk.
    if (is_air_chunk(pos, get_coarse_height(pos)))
        return nullptr;

    return get_or_generate_chunk(pos, terraingen_.size());
}

chunk_ptr
world::get_raw_chunk (chunk_coordinates pos)
{
    return storage_.get_chunk(pos);
}

void
world::store (chunk_coordinates pos, chunk_ptr data)
{
    // Adjust the coarse height map if needed
    map_coordinates mc (pos);
    auto h (get_coarse_height(mc));
    if (needs_chunk_height_adjustment(pos, h))
        store(mc, adjust_chunk_height(pos, h));

    //boost::lock_guard<std::recursive_mutex> chunk_lock (data->lock);

    storage_.store(pos, data);

    // Regenerate surface data
    surface_ptr updated_surface (get_or_create_surface(pos));
    neighborhood<chunk_ptr> nbh (*this, pos);

    updated_surface->opaque      = extract_opaque_surface(nbh);
    updated_surface->transparent = extract_transparent_surface(nbh);

    if (updated_surface->empty())
        trace("STORED EMPTY SURFACE");

    // Update the light maps
    lightmap_ptr lm (get_or_create_lightmap(pos));

    lm->opaque.resize(count_faces(updated_surface->opaque));
    fill(lm->opaque, light());
    if (!updated_surface->opaque.empty())
    {
        for (auto& g : lightgen_)
            g->generate(pos, updated_surface->opaque, lm->opaque, 0);
    }

    lm->transparent.resize(count_faces(updated_surface->transparent));
    fill(lm->transparent, light());
    if (!updated_surface->transparent.empty())
    {
        for (auto& g : lightgen_)
            g->generate(pos, updated_surface->transparent, lm->transparent, 0);
    }

    store(pos, updated_surface);
    store(pos, lm);
}

bool
world::is_chunk_available (chunk_coordinates pos)
{
    auto lock (storage_.acquire_read_lock());
    return storage_.is_chunk_available(pos);
}

lightmap_ptr
world::get_lightmap (chunk_coordinates pos)
{
    lightmap_ptr result;

    // Do we have the lightmap in storage?
    result = storage_.get_lightmap(pos);
    if (result)
        return result;

    // Make sure there's a surface available so we can make one
    auto s (get_surface(pos));
    if (!s || s->empty())
        return result;

    // Create a new lightmap and run it through the generators.
    result.reset(new light_data);

    result->opaque.resize(count_faces(s->opaque));
    if (!s->opaque.empty())
    {
        for (auto& g : lightgen_)
            g->generate(pos, s->opaque, result->opaque, 2);
    }

    result->transparent.resize(count_faces(s->transparent));
    if (!s->transparent.empty())
    {
        for (auto& g : lightgen_)
            g->generate(pos, s->transparent, result->transparent, 2);
    }

    store(pos, result);

    return result;
}

bool
world::is_lightmap_available (chunk_coordinates pos)
{
    auto lock (storage_.acquire_read_lock());
    return storage_.is_lightmap_available(pos);
}

void
world::store (chunk_coordinates pos, lightmap_ptr data)
{
    auto lock (storage_.acquire_write_lock());
    storage_.store(pos, data);
}

compressed_data
world::get_compressed_lightmap (chunk_coordinates xyz)
{
    // Make sure the lightmap was generated first
    if (!is_lightmap_available(xyz))
    {
        auto srf (get_surface(xyz));
        if (srf == nullptr || srf->empty())
            return compressed_data();

        storage_.store(xyz, get_lightmap(xyz));
    }

    return storage_.get_compressed_lightmap(xyz);
}



surface_ptr
world::get_surface (chunk_coordinates pos)
{
    surface_ptr result;

    // Return nothing if it's an all-air chunk.
    auto ch (get_coarse_height(pos));
    if (is_air_chunk(pos, ch))
        return result;

    // Do we have the surface in storage?
    result = storage_.get_surface(pos);
    if (result)
        return result;

    {
    // Lock ordering is important here!  If it's not the same as the
    // order used in voxel_range(), there's a chance of deadlocks.
    /*
    auto locks (lock_region({
                              pos + world_vector( 0, 0,-1),
                              pos + world_vector( 0,-1, 0),
                              pos + world_vector(-1, 0, 0),
                              pos,
                              pos + world_vector( 1, 0, 0),
                              pos + world_vector( 0, 1, 0),
                              pos + world_vector( 0, 0, 1)
                            }));
    */

    neighborhood<chunk_ptr> nbh (*this, pos);
    result.reset(new surface_data(extract_opaque_surface(nbh),
                                  extract_transparent_surface(nbh)));

    if (result->empty())
        trace("GENERATED EMPTY SURFACE %1%", world_rel_coordinates(pos - world_chunk_center));

    }
    assert(result);
    store(pos, result);

    return result;
}

bool
world::is_surface_available (chunk_coordinates pos)
{
    auto lock (storage_.acquire_read_lock());
    return storage_.is_surface_available(pos);
}

void
world::store (chunk_coordinates pos, surface_ptr data)
{
    auto lock (storage_.acquire_write_lock());
    storage_.store(pos, data);
}

compressed_data
world::get_compressed_surface (chunk_coordinates xyz)
{
    // If no surface is available yet, generate a new one from the raw
    // map data.
    if (!is_surface_available(xyz))
    {
        if (!get_surface(xyz))
            return compressed_data();
    }
    return storage_.get_compressed_surface(xyz);
}

//---------------------------------------------------------------------------

chunk_height
world::get_coarse_height (map_coordinates pos)
{
    // Do we have the heightmap in storage?
    if (is_coarse_height_available(pos))
        return storage_.get_coarse_height(pos);

    // Figure out the coarse height.  Every terrain generator knows how
    // to estimate this best.  We'll pick the highest answer.
    bool found (false);
    chunk_height highest (0);
    for (auto& g : terraingen_)
    {
        chunk_height this_height (g->estimate_height(pos));
        if (this_height != undefined_height && this_height > highest)
        {
            highest = this_height;
            found = true;
        }
    }

    if (found)
    {
        trace("Height at %1% estimated to be %2%", pos, int32_t(highest - water_level / chunk_size));
        store(pos, highest);
        return highest;
    }
    else
    {
        trace("WARNING: no height could be estimated for %1%", pos);
    }

    return undefined_height;
}

bool
world::is_coarse_height_available (map_coordinates pos)
{
    auto lock (storage_.acquire_read_lock());
    return storage_.is_coarse_height_available(pos);
}

void
world::store (map_coordinates pos, chunk_height data)
{
    auto lock (storage_.acquire_write_lock());
    storage_.store(pos, data);
}

int
world::find_area_generator (const std::string& name) const
{
    int index (0);
    for (auto i (areagen_.begin()); i != areagen_.end(); ++i, ++index)
    {
        if ((**i).name() == name)
            return index;
    }
    return -1;
}

std::tuple<world_coordinates, world_coordinates>
world::raycast(const wfpos& origin, const yaw_pitch& direction,
               float distance)
{
    typedef std::tuple<world_coordinates, world_coordinates> tuple_type;

    trace("raycasting at %1% towards %2%", origin.pos, direction);

    auto line (voxel_raycast(origin.frac, origin.frac + from_spherical(direction) * distance));
    if (line.size() < 2)
        return tuple_type(origin.pos, origin.pos);

    auto lock (storage_.acquire_read_lock());
    for (auto i (std::next(line.begin())); i != line.end(); ++i)
    {
        auto coll_block (get_block_nolocking(*i + origin.pos));

        // If it's an air block, we can skip the other checks.
        if (coll_block.is_air())
            continue;

        // If it's a normal block, we found an intersection.
        auto& coll_material (material_prop[coll_block.type]);
        if (!coll_material.is_custom_block())
            return tuple_type(origin.pos + *(i-1), origin.pos + *i);

        // It's a custom model; we'll need to do a detailed raycast
        // against every component.
        ray<float> pr ((origin.frac - vector(*i)) * 16.f, direction);
        for (auto& part : coll_material.model)
        {
            auto intsct (ray_box_intersection(pr, aabb<vector>(part.box)));
            if (intsct)
                return tuple_type(origin.pos + *(i-1), origin.pos + *i);
        }
    }

    return tuple_type(origin.pos, origin.pos);
}

block world::get_block (world_coordinates pos)
{
    //auto lock (storage_.acquire_read_lock());
    return get_block_nolocking(pos);
}

block world::get_block_nolocking (world_coordinates pos)
{
    chunk_coordinates cp (pos / chunk_size);
    chunk_ptr cnk (get_chunk(cp));

    if (cnk == nullptr)
    {
        trace("block at %1% is air", pos);
        // Just making sure we were really poking around in an air chunk.
        assert(is_air_chunk(cp, get_coarse_height(cp)));
        return type::air;
    }

    trace("block at %1% is %2%", pos, (*cnk)[pos % chunk_size].type);
    return (*cnk)[pos % chunk_size];
}

void
world::change_block (world_coordinates pos, uint16_t material)
{
    chunk_coordinates cp (pos / chunk_size);
    chunk_index       ci (pos % chunk_size);

    chunk_ptr cnk;
    auto coarse_h (get_coarse_height(cp));

    if (needs_chunk_height_adjustment(cp, coarse_h))
    {
        if (material == 0)
            return; // It's already air, quit

        storage_.store(cp, adjust_chunk_height(cp, coarse_h));
        cnk = get_or_create_chunk(cp);
    }
    else
    {
        cnk = get_chunk(cp);
    }

    {

    if ((*cnk)[ci].type == material)
        return; // Nothing has changed, don't bother.

    (*cnk)[ci].type = material;
    cnk->is_dirty = true;
    storage_.store(cp, cnk);
    update(cp);
    }

    // If this block was changed to air, and it's at the edge of the
    // chunk, we need to update the surface of the neighboring chunk(s)
    // as well.
    if (material == 0)
    {
        if (ci.x == 0)
            update(cp + chunk_coordinates(-1, 0, 0));
        else if (ci.x == chunk_size - 1)
            update(cp + chunk_coordinates(1, 0, 0));

        if (ci.y == 0)
            update(cp + chunk_coordinates(0, -1, 0));
        else if (ci.y == chunk_size - 1)
            update(cp + chunk_coordinates(0, 1, 0));

        if (ci.z == 0)
            update(cp + chunk_coordinates(0, 0, -1));
        else if (ci.z == chunk_size - 1)
            update(cp + chunk_coordinates(0, 0, 1));
    }
}

void
world::update (chunk_coordinates cp)
{
    trace("update chunk %1%", world_rel_coordinates(cp - world_chunk_center));
    // Regenerate surface
    neighborhood<chunk_ptr> nbh (storage_, cp);
    surface_ptr srfc (new surface_data(extract_opaque_surface(nbh),
                                       extract_transparent_surface(nbh)));

    // Regenerate light map
    lightmap_ptr lm (new light_data);
    lm->opaque.resize(count_faces(srfc->opaque));
    if (!srfc->opaque.empty())
    {
        for (auto& g: lightgen_)
            g->generate(cp, srfc->opaque, lm->opaque, 2);
    }

    lm->transparent.resize(count_faces(srfc->transparent));
    if (!srfc->transparent.empty())
    {
        for (auto& g : lightgen_)
            g->generate(cp, srfc->transparent, lm->transparent, 2);
    }

    storage_.store(cp, srfc);
    storage_.store(cp, lm);

    changeset.insert(cp);
}

void
world::change_block (world_coordinates pos, const std::string& material)
{
    trace("change block %1% to %2%", pos , material);
    change_block(pos, find_material(material));
}

void
world::cleanup()
{
    storage_.cleanup();
}

//---------------------------------------------------------------------------

void
world::refine_lightmap (chunk_coordinates pos, int phase)
{
    surface_ptr s (get_surface(pos));
    if (!s)
        return;

    // Create a blank lightmap and run it through the generators.
    lightmap_ptr result (new light_data);
    result->opaque.resize(count_faces(s->opaque));
    if (!s->opaque.empty())
    {
        for (auto& g : lightgen_)
            g->generate(pos, s->opaque, result->opaque, phase);
    }

    result->transparent.resize(count_faces(s->transparent));
    if (!s->transparent.empty())
    {
        for (auto& g : lightgen_)
            g->generate(pos, s->transparent, result->transparent, phase);
    }

    store(pos, result);
}

chunk_ptr
world::get_or_create_chunk(chunk_coordinates pos)
{
    auto result (storage_.get_chunk(pos));

    if (result == nullptr)
    {
        result = std::make_shared<chunk>();

        auto coarse_h (get_coarse_height(pos));
        if (needs_chunk_height_adjustment(pos, coarse_h))
            storage_.store(pos, adjust_chunk_height(pos, coarse_h));
    }

    return result;
}

lightmap_ptr
world::get_or_create_lightmap(chunk_coordinates pos)
{
    auto result (storage_.get_lightmap(pos));
    if (result == nullptr)
        return std::make_shared<light_data>();

    return result;
}

surface_ptr
world::get_or_create_surface(chunk_coordinates pos)
{
    auto result (storage_.get_surface(pos));
    if (result == nullptr)
        return std::make_shared<surface_data>();

    return result;
}

chunk_ptr
world::get_or_generate_chunk(chunk_coordinates pos, int phase, bool adjust_height)
{
    auto result (storage_.get_chunk(pos));

    if (result == nullptr)
    {
        auto coarse_h (get_coarse_height(pos));
        if (needs_chunk_height_adjustment(pos, coarse_h))
        {
            if (!adjust_height)
                return nullptr;

            trace("new air chunk at %1%", world_vector(pos - world_chunk_center));
            result = std::make_shared<chunk>();

            // If this is an air chunk, don't run it through the generators.
            result->generation_phase = phase;
            storage_.store(pos, adjust_chunk_height(pos, coarse_h));
        }
        else
        {
            trace("new blank chunk at %1%", world_vector(pos - world_chunk_center));
            result = std::make_shared<chunk>();
        }
        storage_.store(pos, result);
    }

    if (result->generation_phase < phase)
    {
        trace("running chunk at %1% through terrain generators %2%...",
              world_vector(pos - world_chunk_center), (int)result->generation_phase);

        for (int i (result->generation_phase);
             i < std::min<int>(phase, terraingen_.size()); ++i)
        {
            terraingen_[i]->generate(pos, *result);
        }
        result->generation_phase = phase;
        storage_.store(pos, result);
    }

    return result;
}

world::exclusive_section
world::lock_region(const std::set<chunk_coordinates>& region,
                   const terrain_generator_i& requester)
{
    std::vector<boost::unique_lock<boost::mutex>> locks;

    unsigned int phase(0);
    for (; phase < terraingen_.size(); ++phase)
    {
        if (terraingen_[phase].get() == &requester)
            break;
    }

    if (phase == terraingen_.size())
        throw std::runtime_error("lock was requested by an unregistered terrain generation module");

    // First make sure we have all the locks we need, without actually
    // locking the mutexes yet.  This step might trigger more terrain
    // generation, so this could take a while.
    for (auto cnk_pos : region)
    {
        auto cnk (get_or_generate_chunk(cnk_pos, phase));
        if (cnk)
            locks.emplace_back(cnk->lock, boost::defer_lock);
    }

    // Now try to lock them.  Because we always lock them in the same order,
    // there are no deadlocks if two or more generators contest overlapping
    // parts of the world.
    for (auto& l : locks)
        l.lock();

    exclusive_section result (std::move(locks));
    for (auto cnk : region)
        result.set_chunk(cnk, get_raw_chunk(cnk));

    return result;
}

world::exclusive_section
world::lock_range(const range<chunk_coordinates>& region,
                   const terrain_generator_i& requester)
{
    std::vector<boost::unique_lock<boost::mutex>> locks;

    unsigned int phase(0);
    for (; phase < terraingen_.size(); ++phase)
    {
        if (terraingen_[phase].get() == &requester)
            break;
    }

    if (phase == terraingen_.size())
        throw std::runtime_error("lock was requested by an unregistered terrain generation module");

    // First make sure we have all the locks we need, without actually
    // locking the mutexes yet.  This step might trigger more terrain
    // generation, so this could take a while.
    for (auto cnk_pos : region)
    {
        auto cnk (get_or_generate_chunk(cnk_pos, phase));
        if (cnk)
            locks.emplace_back(cnk->lock, boost::defer_lock);
    }

    // Now try to lock them.  Because we always lock them in the same order,
    // there are no deadlocks if two or more generators contest overlapping
    // parts of the world.
    for (auto& l : locks)
        l.lock();

    exclusive_section result (std::move(locks));
    for (auto cnk : region)
        result.set_chunk(cnk, get_raw_chunk(cnk));

    return result;
}

/*
world::exclusive_section
world::lock_region(std::initializer_list<chunk_coordinates> cnks)
{
    unsigned int phase (terraingen_.size());
    std::vector<boost::unique_lock<std::recursive_mutex>> locks;
bool first(true);
chunk_coordinates prev;

    trace("obtaining locks for individual positions %1%", *cnks.begin());

    for (auto cnk : cnks)
    {
        if (!first)
            assert(cnk > prev);
        first=false;
        prev=cnk;
        auto gc (get_or_generate_chunk(cnk, phase, false));
//        if (gc != nullptr)
//            locks.emplace_back(gc->lock, std::defer_lock);
    }

   // trace((format("mutexing locks for individual positions %1%") % *cnks.begin()).str());

//    for (auto& l : locks)
//        l.lock();

   // trace((format("done, holding locks for individual positions %1%") % *cnks.begin()).str());

    return multi_lock(std::move(locks));
}
*/

world::chunk_type
world::get_type(chunk_coordinates xyz) const
{
    if (!storage_.is_coarse_height_available(xyz))
        return unknown_chunk;

    auto coarse_h (storage_.get_coarse_height(xyz));
    if (coarse_h == undefined_height)
        return unknown_chunk;

    if (is_air_chunk(xyz, coarse_h))
        return air_chunk;

    return normal_chunk;
}

void
world::worker(int id)
{
    trace("launched worker %1%", id);

    while (true)
    {
        auto rq (requests.pop());
        trace("new job type %1% for worker %2%", rq.type, id);

        switch (rq.type)
        {
        case request::quit:
            trace("stopped worker %1%", id);
            return;

        case request::chunk:
            break;

        case request::surface:
            if (!is_surface_available(rq.pos))
                get_surface(rq.pos);

            break;

        case request::lightmap:
            if (!is_lightmap_available(rq.pos))
                get_lightmap(rq.pos);

            break;

        case request::surface_and_lightmap:
            trace("worker %1% checks for surface", id);
            if (!is_surface_available(rq.pos))
            {
                trace("worker %1% generating surface", id);
                get_surface(rq.pos);
                trace("worker %1% done generating surface", id);
            }
            else
            {
                trace("worker %1% found surface", id);
            }
            trace("worker %1% checks for lightmap", id);
            if (!is_lightmap_available(rq.pos))
            {
                trace("worker %1% generating lightmap", id);
                get_lightmap(rq.pos);
                trace("worker %1% done generating lightmap", id);
            }
            else
            {
                trace("worker %1% found lightmap", id);
            }
            break;
        }

        trace("worker %1% signals answer", id);
        rq.answer();
        trace("worker %1% done", id);
    }
}

} // namespace hexa

