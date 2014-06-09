//---------------------------------------------------------------------------
// client/chunk_cache.cpp
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

#include "chunk_cache.hpp"

#include <hexa/lightmap.hpp>
#include <hexa/persistent_storage_i.hpp>
#include <hexa/surface.hpp>

namespace hexa
{

namespace
{

template <typename type>
type unpack_as(const compressed_data& data)
{
    auto tmp(decompress(data));
    return deserialize_as<type>(tmp);
}

} // anonymous namespace

//---------------------------------------------------------------------------

chunk_cache::chunk_cache(persistent_storage_i& store, size_t limit)
    : store_(store)
    , limit_(limit)
{
}

void chunk_cache::cleanup()
{
    {
        std::unique_lock<std::mutex> lock(surfaces_mutex_);
        surfaces_.prune(limit_);
    }
    {
        std::unique_lock<std::mutex> lock(lightmaps_mutex_);
        lightmaps_.prune(limit_);
    }
    {
        std::unique_lock<std::mutex> lock(heights_mutex_);
        heights_.prune(limit_ * 8);
    }
}

bool chunk_cache::is_coarse_height_available(const map_coordinates& pos) const
{
    std::unique_lock<std::mutex> lock(heights_mutex_);
    return heights_.count(pos) != 0 || store_.is_available(pos);
}

chunk_height chunk_cache::get_coarse_height(const map_coordinates& pos)
{
    std::unique_lock<std::mutex> lock(heights_mutex_);

    auto found(heights_.try_get(pos));
    if (found)
        return *found;

    if (!store_.is_available(pos))
        return undefined_height;

    auto value(store_.retrieve(pos));
    return heights_[pos] = value;
}

void chunk_cache::store_coarse_height(const map_coordinates& pos,
                                      chunk_height value)
{
    std::unique_lock<std::mutex> lock(heights_mutex_);
    heights_[pos] = value;
    store_.store(pos, value);
}

//---------------------------------------------------------------------------

bool chunk_cache::is_surface_available(const chunk_coordinates& pos) const
{
    std::unique_lock<std::mutex> lock(surfaces_mutex_);
    return surfaces_.count(pos) != 0
           || store_.is_available(persistent_storage_i::surface, pos);
}

const surface_data& chunk_cache::get_surface(const chunk_coordinates& pos)
{
    std::unique_lock<std::mutex> lock(surfaces_mutex_);

    auto found(surfaces_.try_get(pos));
    if (found)
        return *found;

    assert(store_.is_available(persistent_storage_i::surface, pos));
    auto compr_surf(store_.retrieve(persistent_storage_i::surface, pos));
    return surfaces_[pos] = unpack_as<surface_data>(compr_surf);
}

void chunk_cache::store_surface(const chunk_coordinates& pos,
                                const compressed_data& data)
{
    std::unique_lock<std::mutex> lock(surfaces_mutex_);

    surfaces_.remove(pos);
    store_.store(persistent_storage_i::surface, pos, data);
}

//---------------------------------------------------------------------------

bool chunk_cache::is_lightmap_available(const chunk_coordinates& pos) const
{
    std::unique_lock<std::mutex> lock(lightmaps_mutex_);

    return lightmaps_.count(pos) != 0
           || store_.is_available(persistent_storage_i::light, pos);
}

const light_data& chunk_cache::get_lightmap(const chunk_coordinates& pos)
{
    std::unique_lock<std::mutex> lock(lightmaps_mutex_);

    auto found(lightmaps_.try_get(pos));
    if (found)
        return *found;

    assert(store_.is_available(persistent_storage_i::light, pos));
    auto compr_surf(store_.retrieve(persistent_storage_i::light, pos));
    return lightmaps_[pos] = unpack_as<light_data>(compr_surf);
}

void chunk_cache::store_lightmap(const chunk_coordinates& pos,
                                 const compressed_data& data)
{
    std::unique_lock<std::mutex> lock(lightmaps_mutex_);
    lightmaps_.remove(pos);
    store_.store(persistent_storage_i::light, pos, data);
}

} // namespace hexa
