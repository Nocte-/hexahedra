//---------------------------------------------------------------------------
// hexa/server/world.cpp
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

#include "world.hpp"

#include <boost/range/adaptor/reversed.hpp>

#include <hexa/geometric.hpp>
#include <hexa/ray.hpp>
#include <hexa/trace.hpp>
#include <hexa/voxel_algorithm.hpp>

#include "extract_surface.hpp"
#include "world_subsection.hpp"
#include "world_lightmap_access.hpp"
#include "world_terraingen_access.hpp"

using namespace boost::adaptors;
using namespace boost::range;

constexpr uint8_t finished_phase = 0xff;

namespace hexa {

namespace {

static chunk empty;
static surface_data empty_surface;

template <typename type>
compressed_data pack (const type& data)
{
    return compress(serialize(data));
}

template <typename type>
type unpack_as (const compressed_data& data)
{
    auto tmp (decompress(data));
    return deserialize_as<type>(tmp);
}

} // anonymous namespace

//---------------------------------------------------------------------------

world::world (persistent_storage_i &storage)
    : storage_(storage)
    , seed_(0)
{
    empty.clear();
}

void
world::add_area_generator(std::unique_ptr<area_generator_i>&& gen)
{
    areagen_.emplace_back(std::move(gen));
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

void
world::add_terrain_generator(std::unique_ptr<terrain_generator_i>&& gen)
{
    terraingen_.emplace_back(std::move(gen));
}

void
world::add_lightmap_generator (std::unique_ptr<lightmap_generator_i>&& gen)
{
    lightgen_.emplace_back(std::move(gen));
}

void
world::cleanup()
{
    ///\todo Implement cleanup()
}

//---------------------------------------------------------------------------

const chunk&
world::get_chunk (chunk_coordinates pos)
{
    if (is_air_chunk(pos, get_coarse_height(pos)))
        return empty;

    return get_chunk_writable(pos);
}

chunk&
world::get_chunk_writable (chunk_coordinates pos)
{
    assert(pos.x < chunk_world_limit.x);
    assert(pos.y < chunk_world_limit.y);
    assert(pos.z < chunk_world_limit.z);

    auto found (chunks_.try_get(pos));
    if (found)
        return *found;

    auto& result (chunks_[pos]);
    if (storage_.is_available(persistent_storage_i::chunk, pos))
    {
        auto compressed (storage_.retrieve(persistent_storage_i::chunk, pos));
        result = unpack_as<chunk>(compressed);
    }
    else if (!is_air_chunk(pos, get_coarse_height(pos)))
    {
        result = generate_chunk(pos);
        storage_.store(persistent_storage_i::chunk, pos, pack(result));
    }
    else
    {
        adjust_coarse_height(pos);
        storage_.store(persistent_storage_i::chunk, pos, pack(result));
    }

    return result;
}

const area_data&
world::get_area_data (map_coordinates pos2d, uint16_t index)
{
    assert(pos2d.x < chunk_world_limit.x);
    assert(pos2d.y < chunk_world_limit.y);

    return get_area_data_writable(pos2d, index);
}

area_data&
world::get_area_data_writable (map_coordinates pos2d, uint16_t index)
{
    constexpr auto store_area (persistent_storage_i::area);

    world_coordinates pos (pos2d.x, pos2d.y, index);
    auto i (area_data_.try_get(pos));
    if (i)
        return *i;

    auto& ad (area_data_[pos]);
    if (storage_.is_available(store_area, pos))
    {
        ad = unpack_as<area_data>(storage_.retrieve(store_area, pos));
    }
    else
    {
        if (index >= areagen_.size())
        {
            trace("ERROR: index %1% of %2%", index, areagen_.size());
            throw std::out_of_range("area_data index out of range");
        }

        auto& generator (areagen_[index]);
        ad = generator->generate(pos2d);

        world_terraingen_access proxy (*this);
        for (auto& tg : terraingen_)
            tg->generate(proxy, generator->name(), pos2d, ad);

        if (generator->should_write_to_file())
            storage_.store(store_area, pos, pack(ad));
    }
    return ad;
}

const surface_data&
world::get_surface (chunk_coordinates pos)
{
    assert(pos.x < chunk_world_limit.x);
    assert(pos.y < chunk_world_limit.y);
    assert(pos.z < chunk_world_limit.z);

    if (is_air_chunk(pos, get_coarse_height(pos)))
        return empty_surface;

    constexpr auto store_surface (persistent_storage_i::surface);

    auto i (surfaces_.try_get(pos));
    if (i)
        return *i;

    auto& srf (surfaces_[pos]);
    if (storage_.is_available(store_surface, pos))
    {
        srf = unpack_as<surface_data>(storage_.retrieve(store_surface, pos));
    }
    else
    {
        // Build a surface and store it.
        srf = build_surface(pos);
        storage_.store(store_surface, pos, pack(srf));
    }

    return srf;
}

const light_data&
world::get_lightmap (chunk_coordinates pos)
{
    assert(pos.x < chunk_world_limit.x);
    assert(pos.y < chunk_world_limit.y);
    assert(pos.z < chunk_world_limit.z);

    constexpr auto store_light (persistent_storage_i::light);

    auto i (lightmaps_.try_get(pos));
    if (i)
        return *i;

    auto& lm (lightmaps_[pos]);
    if (storage_.is_available(store_light, pos))
    {
        lm = unpack_as<light_data>(storage_.retrieve(store_light, pos));
    }
    else
    {
        lm = generate_lightmap(pos);
        storage_.store(store_light, pos, pack(lm));
    }
    return lm;
}


chunk_height
world::get_coarse_height (map_coordinates pos)
{
    assert(pos.x < chunk_world_limit.x);
    assert(pos.y < chunk_world_limit.y);

    auto i (coarse_heights_.try_get(pos));
    if (i)
        return *i;

    if (storage_.is_available(pos))
        return coarse_heights_[pos] = storage_.retrieve(pos);

    return set_coarse_height({pos.x, pos.y, generate_coarse_height(pos)});
}

compressed_data
world::get_compressed_surface (chunk_coordinates pos)
{
    if (storage_.is_available(persistent_storage_i::surface, pos))
        return storage_.retrieve(persistent_storage_i::surface, pos);

    auto& srf (get_surface(pos));
    auto result (pack(srf));
    storage_.store(persistent_storage_i::surface, pos, result);

    return result;
}

compressed_data
world::get_compressed_lightmap(chunk_coordinates pos)
{
    if (storage_.is_available(persistent_storage_i::light, pos))
        return storage_.retrieve(persistent_storage_i::light, pos);

    auto& lm (get_lightmap(pos));
    auto result (pack(lm));
    storage_.store(persistent_storage_i::light, pos, result);

    return result;
}


//---------------------------------------------------------------------------


void
world::commit_write (chunk_coordinates pos)
{
    adjust_coarse_height(pos);
    storage_.store(persistent_storage_i::chunk, pos, pack(chunks_.get(pos)));

    // Update the surface and the six surrounding surfaces.
    for (auto rel : neumann_neighborhood)
    {
        auto p (pos + rel);
        if (!is_air_chunk(p, get_coarse_height(p)))
        {
            auto srf (build_surface(p));
            surfaces_[p] = srf;
            storage_.store(persistent_storage_i::surface, p, pack(srf));

            auto lm (generate_lightmap(p));
            lightmaps_[p] = lm;
            storage_.store(persistent_storage_i::light, p, pack(lm));

            on_update_surface(p);
        }
    }
}

world_read
world::acquire_read_access()
{
    return world_read(*this);
}

world_write
world::acquire_write_access (const chunk_coordinates &pos)
{
    world_write proxy (*this);
    proxy.add(pos, get_chunk_writable(pos));
    return proxy;
}

//---------------------------------------------------------------------------

bool
world::is_area_available (map_coordinates pos2d, uint16_t idx) const
{
    chunk_coordinates pos (pos2d.x, pos2d.y, idx);
    return    area_data_.count(pos) != 0
           || storage_.is_available(persistent_storage_i::area, pos);
}

bool
world::is_chunk_available (chunk_coordinates pos) const
{
    return    chunks_.count(pos) != 0
           || storage_.is_available(persistent_storage_i::chunk, pos);
}

bool
world::is_surface_available (chunk_coordinates pos) const
{
    return    surfaces_.count(pos) != 0
           || storage_.is_available(persistent_storage_i::surface, pos);
}

bool
world::is_lightmap_available (chunk_coordinates pos) const
{
    return    lightmaps_.count(pos) != 0
           || storage_.is_available(persistent_storage_i::light, pos);
}

chunk
world::generate_chunk (chunk_coordinates pos)
{
    chunk cnk;
    world_terraingen_access proxy (*this);

    for (auto& step : terraingen_)
        step->generate(proxy, pos, cnk);

    return cnk;
}

light_data&
world::generate_lightmap (chunk_coordinates pos)
{
    world_lightmap_access proxy (*this);
    light_data& result (lightmaps_[pos]);
    auto& surf (get_surface(pos));

    result.opaque.resize(count_faces(surf.opaque));
    if (!result.opaque.empty())
    {
        for (auto& gen : lightgen_)
            gen->generate(proxy, pos, surf.opaque, result.opaque, 1);
    }

    result.transparent.resize(count_faces(surf.transparent));
    if (!result.transparent.empty())
    {
        for (auto& gen : lightgen_)
            gen->generate(proxy, pos, surf.transparent, result.transparent, 1);
    }

    return result;
}

chunk_height
world::generate_coarse_height (map_coordinates pos)
{
    chunk_height result (undefined_height);
    world_terraingen_access data (*this);

    for (auto& g : terraingen_)
    {
        auto s (g->estimate_height(data, pos, result));
        if (s != undefined_height)
        {
            if (result == undefined_height || result < s)
                result = s;
        }
    }

    if (result == undefined_height)
        result = chunk_world_limit.z;

    return result;
}

chunk_height
world::set_coarse_height(chunk_coordinates pos)
{
    coarse_heights_[pos] = pos.z;
    storage_.store(pos, pos.z);
    on_update_coarse_height(pos);
    return pos.z;
}

void
world::adjust_coarse_height (chunk_coordinates pos)
{
    auto current (get_coarse_height(pos));
    if (needs_chunk_height_adjustment(pos, current))
    {
        ++pos.z;
        set_coarse_height(pos);
    }
}

surface_data
world::build_surface (chunk_coordinates pos)
{
    world_subsection_read nbh;

    for (auto rel : neumann_neighborhood)
        nbh.add(rel, get_chunk(pos + rel));

    return surface_data(extract_opaque_surface(nbh),
                        extract_transparent_surface(nbh));
}

//--------------------------------------------------------------------------

std::tuple<world_coordinates, world_coordinates>
raycast(world& w, const wfpos& origin, const yaw_pitch& direction,
        float distance)
{
    typedef std::tuple<world_coordinates, world_coordinates> tuple_type;

    auto line (voxel_raycast(origin.frac, origin.frac + from_spherical(direction) * distance));
    if (line.size() < 2)
        return tuple_type(origin.pos, origin.pos);

    auto proxy (w.acquire_read_access());
    for (auto i (std::next(line.begin())); i != line.end(); ++i)
    {
        world_coordinates pos (*i + origin.pos);
        auto coll_block (proxy.get_chunk(pos >> cnkshift)[pos % chunk_size]);

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

} // namespace hexa

