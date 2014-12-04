//---------------------------------------------------------------------------
/// \file   server/world_read.hpp
/// \brief  Read-only access to the game world
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
#include <boost/optional.hpp>

#include "../basic_types.hpp"
#include "../lightmap.hpp"
#include <hexa/surface.hpp>
#include <hexa/read_write_lockable.hpp>

namespace hexa
{

class area_data;
class chunk;
class compressed_data;
class world;

/** This object grants read access to the game world. */
class world_read
{
    friend class world;

protected:
    world_read(world& w);

public:
    world_read(const world_read&) = delete;

#ifdef _MSC_VER
#else
    world_read(world_read&&) = default;
#endif

public:
    uint16_t get_block(world_coordinates pos);

    const area_data& get_area_data(map_coordinates pos, uint16_t index);

    boost::optional<const area_data&> try_get_area_data(map_coordinates pos,
                                                        uint16_t index);

    const chunk& get_chunk(chunk_coordinates pos);

    const surface_data& get_surface(chunk_coordinates pos);

    light_data get_lightmap(chunk_coordinates pos);

    chunk_height get_coarse_height(map_coordinates pos);

    compressed_data get_compressed_surface(chunk_coordinates pos);

    compressed_data get_compressed_lightmap(chunk_coordinates pos);

    bool is_area_available(map_coordinates pos, uint16_t index) const;
    bool is_chunk_available(chunk_coordinates pos) const;
    bool is_surface_available(chunk_coordinates pos) const;
    bool is_lightmap_available(chunk_coordinates pos) const;

    bool is_air_chunk(chunk_coordinates pos) const;

private:
    world& w_;
    std::unique_lock<std::mutex> lock_;
};

} // namespace hexa
