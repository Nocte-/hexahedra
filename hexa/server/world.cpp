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
#include <boost/range/adaptor/map.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <hexa/geometric.hpp>
#include <hexa/log.hpp>
#include <hexa/memory_cache.hpp>
#include <hexa/surface.hpp>
#include <hexa/neighborhood.hpp>
#include <hexa/trace.hpp>
#include <hexa/voxel_algorithm.hpp>

#include <iostream>

using namespace boost::adaptors;
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

void world::add_area_generator(std::unique_ptr<area_generator_i>&& gen)
{
    areagen_.emplace_back(std::move(gen)); // move needed for gcc 4.7
}

void world::add_terrain_generator(std::unique_ptr<terrain_generator_i>&& gen)
{ 
    terraingen_.emplace_back(std::move(gen));

    std::set<world_vector> sum { world_vector::zero() };
    phases_.clear();

    for (auto& t : terraingen_ | reversed)
    {
        std::set<vector3<int>> inverse;
        for (auto& e : t->span()) inverse.insert(-e);

        auto span_sq (minkowski_sum(t->span(), inverse));
        sum = minkowski_sum(sum, span_sq);
        for (auto s : sum)
            ++phases_[s];
    }
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
    chunk_ptr result;

    // Return nothing if it's an all-air chunk.
    //if (is_air(pos))
    //{
    //    trace("chunk %1% is air", world_vector(pos - world_chunk_center));
    //    return nullptr;
    //}

    {
    trace("acquire read lock");
    auto rl (storage_.acquire_read_lock());
    trace("got read lock");
    result = storage_.get_chunk(pos);
    }
    if (result != nullptr && result->generation_phase == terraingen_.size())
    {
        trace("return chunk %1% from storage", world_vector(pos - world_chunk_center));
        return result;
    }

    trace("generating chunk %1%", world_vector(pos - world_chunk_center));

    std::set<chunk_coordinates> lock_area;
    for (auto offset : phases_ | map_keys)
    {
        trace("locking %1%", offset);
        lock_area.insert(pos + offset);
    }

    auto locked (lock_chunks(lock_area));
    unsigned int phase (0);
    for (auto& generator : terraingen_)
    {
        trace("phase %1%", phase);
        for (auto& abspos : locked | map_keys)
        {
            auto& cnk (locked.get_chunk(abspos));
            if (   cnk.generation_phase == phase
                && lookup(phases_, world_vector(abspos - pos)) > phase)
            {
                trace("- generating chunk %1%", world_vector(abspos - pos));

                auto s (generator->span());
                world_subsection<chunk_ptr> ss;
                for (auto p : s)
                {
                    auto ap (abspos + p);
                    if (locked.has_chunk(ap))
                        ss.set_chunk(ap, locked.get_ptr(ap));
                }

                if (s.size() == 1)
                {
                    trace("  - generating single chunk %1% phase %2%", world_vector(abspos - pos), phase);
                    generator->generate(abspos, cnk);
                    cnk.generation_phase = phase + 1;
                }
                else if (ss.size() == s.size())
                {
                    trace("  - generating multi chunk %1% phase %2%", world_vector(abspos - pos), phase);
                    generator->generate(abspos, ss);
                    cnk.generation_phase = phase + 1;
                }

                trace("  - generating chunk %1% phase %2% done", world_vector(abspos - pos), phase);
            }
            else
            {
                trace("skipping chunk %1%", world_vector(abspos - pos));
            }
        }
        ++phase;
    }

    {
    trace("acquire write lock");
    auto wl (storage_.acquire_write_lock());
    trace("got lock, store chunks");
    storage_.store(locked);
    }

    trace("done, adjusting heights");
    for (auto& l : locked)
    {
        auto coarse_h (get_coarse_height(l.first));
        if (l.first.z >= coarse_h && !l.second->is_air())
        {
            trace("  adjust height at %1% to %2%", l.first, coarse_h);
            set_coarse_height(l.first, coarse_h);
        }
    }

    trace("done, return chunk %1%",  world_vector(pos - world_chunk_center));
    return result;
}

chunk_ptr
world::get_raw_chunk (chunk_coordinates pos)
{
    return storage_.get_chunk(pos);
}

void
world::store (chunk_coordinates pos, chunk_ptr data)
{
    trace("store chunk %1%", pos);

    // Adjust the coarse height map if needed
    map_coordinates mc (pos);
    auto h (get_coarse_height(mc));
    if (needs_chunk_height_adjustment(pos, h))
        set_coarse_height(pos, h);

    {
    trace("store chunk %1%, acquire write lock", pos);
    auto lock (storage_.acquire_write_lock());
    trace("store chunk %1% write lock acquired", pos);
    storage_.store(pos, data);
    }

    if (   data->generation_phase == terraingen_.size()
        && is_surface_available(pos))
    {
        // If this is a fully generated chunk, regenerate the surface data
        surface_ptr updated_surface (get_or_create_surface(pos));
        neighborhood<chunk_ptr> nbh (*this, pos);

        updated_surface->opaque      = extract_opaque_surface(nbh);
        updated_surface->transparent = extract_transparent_surface(nbh);

        // Update the light maps
        lightmap_ptr lm (get_or_create_lightmap(pos));

        lm->opaque.resize(count_faces(updated_surface->opaque));
        fill(lm->opaque, light());
        if (!updated_surface->opaque.empty())
        {
            for (auto& g : lightgen_)
                g->generate(pos, updated_surface->opaque, lm->opaque, 1);
        }

        lm->transparent.resize(count_faces(updated_surface->transparent));
        fill(lm->transparent, light());
        if (!updated_surface->transparent.empty())
        {
            for (auto& g : lightgen_)
                g->generate(pos, updated_surface->transparent, lm->transparent, 1);
        }

        {
        trace("store chunk surface/lightmap %1%, acquire write lock", pos);
        auto lock (storage_.acquire_write_lock());
        trace("store chunk surface/lightmap %1% write lock acquired", pos);
        storage_.store(pos, updated_surface);
        storage_.store(pos, lm);

        if (updated_surface->empty())
        {
            trace("STORED EMPTY SURFACE");
            if (h == pos.z + 1)
                set_coarse_height(pos, h - 1);
        }

        {
        boost::lock_guard<boost::mutex> cs_lock (changeset_lock);
        changeset.insert(pos);
        }

        }
    }

    trace("store chunk %1% done", pos);
}

bool
world::is_chunk_available (chunk_coordinates pos)
{
    trace("acquire read lock");
    auto lock (storage_.acquire_read_lock());
    trace("got read lock");
    return storage_.is_chunk_available(pos);
}

lightmap_ptr
world::get_lightmap (chunk_coordinates pos)
{
    trace("get lightmap %1%", pos);

    // Do we have the lightmap in storage?
    lightmap_ptr result;
    {
    trace("get lightmap %1%, read lock", pos);
    auto lock (storage_.acquire_read_lock());
    trace("get lightmap %1%, read lock acquired", pos);

    result = storage_.get_lightmap(pos);
    if (result)
    {
        trace("return lightmap %1% from storage", pos);
        return result;
    }
    }

    // Make sure there's a surface available so we can make one
    auto s (get_surface(pos));
    if (!s)
        return nullptr;

    // Create a new lightmap and run it through the generators.
    trace("generate lightmap %1%", pos);

    result.reset(new light_data);
    result->opaque.resize(count_faces(s->opaque));
    if (!s->opaque.empty())
    {
        for (auto& g : lightgen_)
            g->generate(pos, s->opaque, result->opaque, 1);
    }

    result->transparent.resize(count_faces(s->transparent));
    if (!s->transparent.empty())
    {
        for (auto& g : lightgen_)
            g->generate(pos, s->transparent, result->transparent, 1);
    }

    trace("store new lightmap %1%", pos);
    store(pos, result);

    return result;
}

bool
world::is_lightmap_available (chunk_coordinates pos)
{
    trace("acquire read lock");
    auto lock (storage_.acquire_read_lock());
    trace("got read lock");
    return storage_.is_lightmap_available(pos);
}

void
world::store (chunk_coordinates pos, lightmap_ptr data)
{
    trace("acquire write lock");
    auto lock (storage_.acquire_write_lock());
    trace("got write lock, storing data");
    storage_.store(pos, data);
}

compressed_data
world::get_compressed_lightmap (chunk_coordinates xyz)
{
    trace("get compressed lightmap %1%", xyz);

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
    //trace("get_surface %1%", world_vector(pos - world_chunk_center));

    surface_ptr result;

    // Return nothing if it's an all-air chunk.
    auto ch (get_coarse_height(pos));
    if (hexa::is_air_chunk(pos, ch))
    {
        //trace("%1% is an air chunk, returning nothing", world_vector(pos - world_chunk_center));
        return result;
    }

    // Do we have the surface in storage?
    result = storage_.get_surface(pos);
    if (result)
    {
        //trace("returning surface %1% from storage", world_vector(pos - world_chunk_center));
        return result;
    }


    trace("acquiring locks around %1% for surface", world_vector(pos - world_chunk_center));

    {

    std::set<chunk_coordinates> neumann;
    neumann.insert(pos);
    for (auto v : dir_vector)
        neumann.insert(pos + world_vector(v));

    for (auto p : neumann)
        get_chunk(p);

    //auto locks (lock_chunks(neumann));

    for (auto p : neumann)
    {
        auto cnk (get_chunk(p));
        if (cnk == nullptr)
            trace("ERROR: chunk %1% is null", p);

        else if (cnk->generation_phase != this->terraingen_.size())
            trace("ERROR: chunk %1% is not fully generated", p);

        else if (cnk->is_air())
            trace("WARNING: chunk %1% is an air chunk", p);
    }

    neighborhood<chunk_ptr> nbh (*this, pos);

    trace("got terrain, generating surface at %1%", world_vector(pos - world_chunk_center));
    result.reset(new surface_data(extract_opaque_surface(nbh),
                                  extract_transparent_surface(nbh)));
    }

    if (result->empty())
    {
        trace("GENERATED EMPTY SURFACE %1%", world_rel_coordinates(pos - world_chunk_center));
        if (ch == pos.z + 1)
        {
            trace("lowering ceiling");
            set_coarse_height(pos, pos.z);
            return nullptr;
        }
    }

    store(pos, result);
    trace("surface %1% is done, returning", world_vector(pos - world_chunk_center));

    return result;
}

bool
world::is_surface_available (chunk_coordinates pos)
{
    trace("acquire read lock");
    auto lock (storage_.acquire_read_lock());
    trace("got read lock");
    return storage_.is_surface_available(pos);
}

void
world::store (chunk_coordinates pos, surface_ptr data)
{
    trace("store surface %1%, acquire write lock", world_vector(pos - world_chunk_center));
    auto lock (storage_.acquire_write_lock());
    storage_.store(pos, data);
    trace("store surface %1% done", world_vector(pos - world_chunk_center));
}

compressed_data
world::get_compressed_surface (chunk_coordinates xyz)
{
    trace("get compressed surface %1%", xyz);

    // If no surface is available yet, generate a new one from the raw
    // map data.
    if (!is_surface_available(xyz))
    {
        if (!get_surface(xyz))
        {
            trace("get compressed surface failed");
            return compressed_data();
        }
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
    chunk_height highest (undefined_height);
    for (auto& g : terraingen_)
    {
        chunk_height this_height (g->estimate_height(pos, highest));
        if (   this_height != undefined_height
            && (highest == undefined_height || this_height > highest))
        {
            highest = this_height;
            found = true;
        }
    }

    if (found)
    {
        trace("Height at %1% estimated to be %2%",
              map_rel_coordinates(pos - map_chunk_center),
              int32_t(highest - water_level / chunk_size));
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
    trace("acquire read lock");
    auto lock (storage_.acquire_read_lock());
    trace("got read lock");
    return storage_.is_coarse_height_available(pos);
}

void
world::store (map_coordinates pos, chunk_height data)
{
    trace("acquire write lock");
    auto lock (storage_.acquire_write_lock());
    trace("got write lock, storing height data");
    storage_.store(pos, data);
}

bool
world::is_air (chunk_coordinates xyz)
{
    return is_air_chunk(xyz, get_coarse_height(xyz));
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

    trace("acquire read lock");
    auto lock (storage_.acquire_read_lock());
    trace("got read lock");
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
            auto intsct (ray_box_intersection(pr, part.bounding_box()));
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
    if (is_air_chunk(cp, get_coarse_height(cp)))
        return type::air;

    chunk_ptr cnk (get_chunk(cp));
    if (cnk == nullptr)
    {
        trace("block at %1% is air", pos);
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

    trace("change block at %1% to %2%", pos, material);

    chunk_ptr cnk;
    auto coarse_h (get_coarse_height(cp));

    if (needs_chunk_height_adjustment(cp, coarse_h))
    {
        if (material == 0)
            return; // It's already air, quit

        set_coarse_height(cp, coarse_h);
        cnk = get_or_create_chunk(cp);
    }
    else
    {
        trace("coarse height map %1% is still OK (is %2%)",
              map_rel_coordinates(cp - map_chunk_center),
              coarse_h);
        cnk = get_chunk(cp);
    }

    if ((*cnk)[ci].type == material)
        return; // Nothing has changed, don't bother.

    (*cnk)[ci].type = material;
    cnk->is_dirty = true;
    storage_.store(cp, cnk);
    update(cp);

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
    neighborhood<chunk_ptr> nbh (storage_, cp);
    if (nbh.center() == nullptr || nbh.center()->generation_phase != terraingen_.size())
        return;

    // Regenerate surface
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

    {
        boost::lock_guard<boost::mutex> lock(changeset_lock);
        changeset.insert(cp);
    }
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
    trace("acquire write lock");
    auto storage_lock (storage_.acquire_write_lock());
    trace("got write lock, creating chunk");

    auto result (storage_.get_chunk(pos));

    if (result == nullptr)
    {
        trace("check+add %1% (a)", pos);
        boost::lock_guard<boost::mutex> locked (check_lock_);
        assert(check_.count(pos) == 0);
        check_.insert(pos);

        result = std::make_shared<chunk>();
        storage_.store(pos, result);

        auto coarse_h (get_coarse_height(pos));
        if (needs_chunk_height_adjustment(pos, coarse_h))
            set_coarse_height(pos, coarse_h);
    }

    return result;
}

lightmap_ptr
world::get_or_create_lightmap(chunk_coordinates pos)
{
    auto result (storage_.get_lightmap(pos));
    if (result == nullptr)
    {
        result = std::make_shared<light_data>();
        storage_.store(pos, result);
    }
    return result;
}

surface_ptr
world::get_or_create_surface(chunk_coordinates pos)
{
    auto result (storage_.get_surface(pos));
    if (result == nullptr)
    {
        result = std::make_shared<surface_data>();
        storage_.store(pos, result);
    }
    return result;
}

void
world::generate_terrain(chunk_coordinates pos, const chunk_ptr& chunk)
{
    generate_terrain(pos, chunk, terraingen_.size());
}

void
world::generate_terrain(chunk_coordinates pos, const chunk_ptr& chunk, int phase)
{
    phase = std::min<int>(phase, terraingen_.size());
    if (chunk->generation_phase >= phase)
    {
        trace("not generating chunk at %1%, is at phase %2%, requested %3%",
              world_vector(pos - world_chunk_center), (int)chunk->generation_phase, phase);

        return;
    }

    trace("running chunk at %1% through terrain generators %2%..%3%",
          world_vector(pos - world_chunk_center), (int)chunk->generation_phase, phase - 1);

    for (int i (chunk->generation_phase); i < phase; ++i)
    {
        trace("running chunk at %1% through terrain generator %2%",
              world_vector(pos - world_chunk_center), i);

        terraingen_[i]->generate(pos, *chunk);
        chunk->generation_phase = i + 1;
    }

    trace("running chunk at %1% through terrain generators %2%, %3% done, storing",
          world_vector(pos - world_chunk_center), (int)chunk->generation_phase, phase);

    chunk->generation_phase = phase;
    store(pos, chunk);
    trace("running chunk at %1% through terrain generators %2%, %3% returning",
          world_vector(pos - world_chunk_center), (int)chunk->generation_phase, phase);
}

unsigned int
world::find_generator (const terrain_generator_i& requester) const
{
    unsigned int phase(0);
    for (; phase < terraingen_.size(); ++phase)
    {
        if (terraingen_[phase].get() == &requester)
            break;
    }

    if (phase == terraingen_.size())
        throw std::runtime_error("lock was requested by an unregistered terrain generation module");

    return phase;
}


locked_subsection
world::lock_chunks (const std::set<chunk_coordinates>& region)
{
    locked_subsection result;

    trace("acquire write lock");
    auto storage_lock (storage_.acquire_write_lock());
    trace("got write lock");

    for (auto cnk_pos : region)
    {
        auto cnk_ptr (storage_.get_chunk(cnk_pos));
        if (!cnk_ptr) // && !is_air(cnk_pos))
        {
            trace("check+add %1% (b)", cnk_pos);

            boost::lock_guard<boost::mutex> locked (check_lock_);
            assert(check_.count(cnk_pos) == 0);
            check_.insert(cnk_pos);

            cnk_ptr.reset(new chunk);
            storage_.store(cnk_pos, cnk_ptr);
        }
        result.add(cnk_pos, cnk_ptr);
    }

    return result;
}

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
world::set_coarse_height(chunk_coordinates cp, chunk_height coarse_h)
{
    auto old_h (get_coarse_height(cp));
    if (old_h == coarse_h)
        return;

    trace("setting coarse height map %1%", coarse_h);
    storage_.store(map_coordinates(cp), coarse_h);
    {
        boost::lock_guard<boost::mutex> lock (height_changeset_lock);
        height_changeset.insert(cp);
    }
}

void
world::worker(int id)
{
    trace("launched worker %1%", id);

    while (true)
    {
        auto rq (requests.pop());
        trace("new job type %1% for worker %2%", rq.type, id);

        try
        {
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
                trace("worker %1% checks for lightmap at %2%", id, world_vector(rq.pos - world_chunk_center));
                if (!is_lightmap_available(rq.pos))
                {
                    trace("worker %1% generates lightmap at %2%", id, world_vector(rq.pos - world_chunk_center));
                    get_lightmap(rq.pos);
                    trace("worker %1% generates lightmap at %2% done", id, world_vector(rq.pos - world_chunk_center));
                }

                break;

            case request::surface_and_lightmap:
                trace("worker %1% checks for surface %2%", id, world_vector(rq.pos - world_chunk_center));
                if (!is_surface_available(rq.pos))
                {
                    trace("worker %1% generating surface %2%", id, world_vector(rq.pos - world_chunk_center));
                    get_surface(rq.pos);
                    trace("worker %1% done generating surface %2%", id, world_vector(rq.pos - world_chunk_center));
                }
                else
                {
                    trace("worker %1% found surface %2%", id, world_vector(rq.pos - world_chunk_center));
                }
                trace("worker %1% checks for lightmap %2%", id, world_vector(rq.pos - world_chunk_center));
                if (!is_lightmap_available(rq.pos))
                {
                    trace("worker %1% generating lightmap %2%", id, world_vector(rq.pos - world_chunk_center));
                    get_lightmap(rq.pos);
                    trace("worker %1% done generating lightmap %2%", id, world_vector(rq.pos - world_chunk_center));
                }
                else
                {
                    trace("worker %1% found lightmap %2%", id, world_vector(rq.pos - world_chunk_center));
                }
                break;
            }

            trace("worker %1% signals answer", id);
            rq.answer();
            trace("worker %1% done", id);
        }
        catch (std::exception& e)
        {
            log_msg("worker %1% caught exception: %2%", id, e.what());
        }
    }
}

} // namespace hexa

