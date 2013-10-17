//---------------------------------------------------------------------------
// lib/memory_cache.cpp
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

#include "memory_cache.hpp"

#include <set>
#include <boost/format.hpp>
#include "log.hpp"
#include "trace.hpp"

namespace hexa {

namespace {

static const surface_ptr empty_surface = surface_ptr(new surface_data);

template <class type>
std::shared_ptr<type> unpack_as(const compressed_data& data)
{
    auto tmp (decompress(data));
    return std::make_shared<type>(deserialize_as<type>(tmp));
}

template <typename ptr>
bool is_not_in_use (ptr p)
{
    return p.unique();
}

} // anonymous namespace

memory_cache::~memory_cache()
{
    cleanup();
}

void
memory_cache::cleanup ()
{
    {
    boost::lock_guard<boost::mutex> areas_lock (areas_mutex_);
    for (auto& p : areas_dirty_)
        next_.store(persistent_storage_i::area, p, compress(serialize(*areas_.get(p))));

    areas_dirty_.clear();
    //areas_.prune_if(size_limit_, is_not_in_use<area_ptr>);
    areas_.prune(size_limit_);
    }
    {
    boost::lock_guard<boost::mutex> chunks_lock (chunks_mutex_);
    std::set<chunk_coordinates> helgrind;
    std::copy(chunks_dirty_.begin(), chunks_dirty_.end(), std::inserter(helgrind, helgrind.begin()));
    for (auto& p : helgrind) //chunks_dirty_)
    {
        boost::lock_guard<boost::mutex> lock (chunks_.get(p)->lock());
        next_.store(persistent_storage_i::chunk, p, compress(serialize(*chunks_.get(p))));
    }
    chunks_dirty_.clear();
    //chunks_.prune_if(size_limit_, is_not_in_use<chunk_ptr>);
    chunks_.prune(size_limit_);
    }
    {
    boost::lock_guard<boost::mutex> lightmaps_lock (lightmaps_mutex_);
    for (auto& p : lightmaps_dirty_)
        next_.store(persistent_storage_i::light, p, compress(serialize(*lightmaps_.get(p))));

    lightmaps_dirty_.clear();
    //lightmaps_.prune_if(size_limit_, is_not_in_use<lightmap_ptr>);
    lightmaps_.prune(size_limit_);
    }
    {
    boost::lock_guard<boost::mutex> surfaces_lock (surfaces_mutex_);
    for (auto& p : surfaces_dirty_)
        next_.store(persistent_storage_i::surface, p, compress(serialize(*surfaces_.get(p))));

    surfaces_dirty_.clear();
    //surfaces_.prune_if(size_limit_, is_not_in_use<surface_ptr>);
    surfaces_.prune(size_limit_);
    }
    {
    boost::lock_guard<boost::mutex> heights_lock (heights_mutex_);
    heights_.prune(size_limit_,
        [&](map_coordinates xy, chunk_height h)
        {
            next_.store(xy, h);
        });
    }
}


void
memory_cache::store (map_coordinates xy, uint16_t index, area_ptr data)
{
    if (!data)
        throw std::logic_error("storing a null area");

    //trace((boost::format("store area at %1%, index %2%") % xy % index).str());

    chunk_coordinates c (xy.x, xy.y, index);

    boost::lock_guard<boost::mutex> areas_lock (areas_mutex_);
    areas_dirty_.insert(c);
    areas_[c] = data;
}

void
memory_cache::store (chunk_coordinates xyz, chunk_ptr data)
{
    if (!data)
        throw std::logic_error("storing a null chunk");

    //trace((boost::format("store chunk at %1%") % world_rel_coordinates(xyz -world_chunk_center)).str());

    boost::lock_guard<boost::mutex> chunks_lock (chunks_mutex_);
    chunks_dirty_.insert(xyz);
    chunks_[xyz] = data;
}

void
memory_cache::store (chunk_coordinates xyz, lightmap_ptr data)
{
    if (!data)
        throw std::logic_error("storing a null lightmap");

    //trace((boost::format("store lightmap at %1%") % world_rel_coordinates(xyz -world_chunk_center)).str());

    boost::lock_guard<boost::mutex> lightmaps_lock (lightmaps_mutex_);
    lightmaps_dirty_.insert(xyz);
    lightmaps_[xyz] = data;
}

void
memory_cache::store (chunk_coordinates xyz, surface_ptr data)
{
    if (!data)
        throw std::logic_error("storing a null surface");

    //trace((boost::format("store surface at %1%") % world_rel_coordinates(xyz -world_chunk_center)).str());

    boost::lock_guard<boost::mutex> surfaces_lock (surfaces_mutex_);
    surfaces_dirty_.insert(xyz);
    surfaces_[xyz] = data;
}

void
memory_cache::store (map_coordinates xy, chunk_height data)
{
    if (data == undefined_height)
        throw std::logic_error("storing an undefined height");

    //trace((boost::format("store height at %1% : %2%")
    //       % map_rel_coordinates(xy - map_chunk_center)
    //       % int32_t(data - world_chunk_center.z)).str());

    boost::lock_guard<boost::mutex> heights_lock (heights_mutex_);
    heights_[xy] = data;
}

void
memory_cache::store (const locked_subsection& region)
{
    for (auto& p : region)
    {
        chunks_dirty_.insert(p.first);
        chunks_[p.first] = p.second;
    }
}


bool
memory_cache::is_area_data_available (map_coordinates xy, uint16_t index)
{
    boost::lock_guard<boost::mutex> areas_lock (areas_mutex_);
    chunk_coordinates xyz (xy.x, xy.y, index);
    return    areas_.count(xyz) > 0
           || next_.is_available(persistent_storage_i::area, xyz);
}

bool
memory_cache::is_chunk_available (chunk_coordinates xyz)
{
    boost::lock_guard<boost::mutex> chunks_lock (chunks_mutex_);
    return    chunks_.count(xyz) > 0
           || next_.is_available(persistent_storage_i::chunk, xyz);
}

bool
memory_cache::is_lightmap_available (chunk_coordinates xyz)
{
    boost::lock_guard<boost::mutex> lightmaps_lock (lightmaps_mutex_);
    return    lightmaps_.count(xyz) > 0
           || next_.is_available(persistent_storage_i::light, xyz);
}

bool
memory_cache::is_surface_available (chunk_coordinates xyz)
{
    boost::lock_guard<boost::mutex> surfaces_lock (surfaces_mutex_);
    return    surfaces_.count(xyz) > 0
           || next_.is_available(persistent_storage_i::surface, xyz);
}

bool
memory_cache::is_coarse_height_available (map_coordinates xy)
{
    boost::lock_guard<boost::mutex> heights_lock (heights_mutex_);
    return    heights_.count(xy) > 0
           || next_.is_available(persistent_storage_i::height, xy);
}

area_ptr
memory_cache::get_area_data (map_coordinates xy, uint16_t index)
{
    chunk_coordinates xyz (xy.x, xy.y, index);

    boost::lock_guard<boost::mutex> areas_lock (areas_mutex_);
    auto found (areas_.try_get(xyz));
    if (found)
        return *found;

    areas_dirty_.erase(xyz);

    if (!next_.is_available(persistent_storage_i::area, xyz))
        return nullptr;

    auto compressed (next_.retrieve(persistent_storage_i::area, xyz));
    auto result (unpack_as<area_data>(compressed));
    areas_[xyz] = result;

    return result;
}

chunk_ptr
memory_cache::get_chunk (chunk_coordinates xyz)
{
    boost::lock_guard<boost::mutex> chunks_lock (chunks_mutex_);
    auto found (chunks_.try_get(xyz));
    if (found)
        return *found;

    chunks_dirty_.erase(xyz);

    if (!next_.is_available(persistent_storage_i::chunk, xyz))
        return nullptr;

    auto result (unpack_as<chunk>(next_.retrieve(persistent_storage_i::chunk, xyz)));
    chunks_[xyz] = result;

    return result;
}

lightmap_ptr
memory_cache::get_lightmap (chunk_coordinates xyz)
{
    boost::lock_guard<boost::mutex> lightmaps_lock (lightmaps_mutex_);

    auto found (lightmaps_.try_get(xyz));
    if (found)
        return *found;

    lightmaps_dirty_.erase(xyz);

    if (!next_.is_available(persistent_storage_i::light, xyz))
        return nullptr;

    auto compressed (next_.retrieve(persistent_storage_i::light, xyz));
    auto result (unpack_as<light_data>(compressed));
    lightmaps_[xyz] = result;

    return result;
}

surface_ptr
memory_cache::get_surface (chunk_coordinates xyz)
{
    boost::lock_guard<boost::mutex> surfaces_lock (surfaces_mutex_);

    auto found (surfaces_.try_get(xyz));
    if (found)
        return *found;

    surfaces_dirty_.erase(xyz);

    if (!next_.is_available(persistent_storage_i::surface, xyz))
        return nullptr;

    auto compressed (next_.retrieve(persistent_storage_i::surface, xyz));
    auto result (unpack_as<surface_data>(compressed));
    surfaces_[xyz] = result;

    return result;
}

chunk_height
memory_cache::get_coarse_height (map_coordinates xy)
{
    boost::lock_guard<boost::mutex> heights_lock (heights_mutex_);

    auto found (heights_.try_get(xy));
    if (!found)
    {
        if (!next_.is_available(persistent_storage_i::height, xy))
            return undefined_height;

        chunk_height result (next_.retrieve(xy));
        heights_[xy] = result;

        return result;
    }

    return *found;
}

compressed_data
memory_cache::get_compressed_lightmap (chunk_coordinates xyz)
{
    boost::lock_guard<boost::mutex> lightmaps_lock (lightmaps_mutex_);

    if (   lightmaps_dirty_.count(xyz) > 0
        || !next_.is_available(persistent_storage_i::light, xyz))
    {
        auto found (lightmaps_.try_get(xyz));
        if (!found)
        {
            // There's a chance the lightmap isn't stored becuase there is
            // no corresponding surface.  In that case, return an empty
            // buffer.
            auto srf (surfaces_.try_get(xyz));
            if (!srf || *srf == nullptr || (**srf).empty())
                return compressed_data();

            // If that's not the case, something's wrong and we can't fix
            // it here.
            throw not_in_storage_error((boost::format("light map at %1%") % xyz).str());
        }

        assert(*found != nullptr);
        lightmaps_dirty_.erase(xyz);
        auto tmp (serialize(**found));
        compressed_data result (compress(tmp));

        next_.store(persistent_storage_i::light, xyz, result);

        return result;
    }

    return next_.retrieve(persistent_storage_i::light, xyz);
}

compressed_data
memory_cache::get_compressed_surface (chunk_coordinates xyz)
{
    boost::lock_guard<boost::mutex> surfaces_lock (surfaces_mutex_);

    if (   surfaces_dirty_.count(xyz) > 0
        || !next_.is_available(persistent_storage_i::surface, xyz))
    {
        auto found (surfaces_.try_get(xyz));
        if (!found)
            throw not_in_storage_error((boost::format("surface at %1%") % xyz).str());

        surfaces_dirty_.erase(xyz);
        auto s_ptr (*found);
        if (!s_ptr)
            throw not_in_storage_error((boost::format("surface at %1% inconsistent") % xyz).str());

        auto tmp (serialize(*s_ptr));
        auto result (compress(tmp));
        next_.store(persistent_storage_i::surface, xyz, result);

        return result;
    }

    return next_.retrieve(persistent_storage_i::surface, xyz);
}

} // namespace hexa

