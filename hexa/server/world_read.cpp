//---------------------------------------------------------------------------
// server/world_read.cpp
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

#include "world_read.hpp"

#include "../chunk.hpp"
#include "../compression.hpp"
#include "../lightmap.hpp"
#include "../surface.hpp"

#include "world.hpp"

namespace hexa
{

world_read::world_read(world& w)
    : w_(w)
    , lock_(w_.single_lock)
{
}

uint16_t world_read::get_block(world_coordinates pos)
{
    return get_chunk(pos >> cnkshift)[pos % chunk_size];
}

const area_data& world_read::get_area_data(map_coordinates pos, uint16_t index)
{
    return w_.get_area_data(pos, index);
}

const chunk& world_read::get_chunk(chunk_coordinates pos)
{
    return w_.get_chunk(pos);
}

const surface_data& world_read::get_surface(chunk_coordinates pos)
{
    return w_.get_surface(pos);
}

compressed_data world_read::get_compressed_surface(chunk_coordinates pos)
{
    return w_.get_compressed_surface(pos);
}

compressed_data world_read::get_compressed_lightmap(chunk_coordinates pos)
{
    return w_.get_compressed_lightmap(pos);
}

chunk_height world_read::get_coarse_height(map_coordinates pos)
{
    return w_.get_coarse_height(pos);
}

bool world_read::is_area_available(map_coordinates pos, uint16_t index) const
{
    return w_.is_area_available(pos, index);
}

bool world_read::is_chunk_available(chunk_coordinates pos) const
{
    return w_.is_chunk_available(pos);
}

bool world_read::is_surface_available(chunk_coordinates pos) const
{
    return w_.is_surface_available(pos);
}

bool world_read::is_lightmap_available(chunk_coordinates pos) const
{
    return w_.is_lightmap_available(pos);
}

bool world_read::is_air_chunk(chunk_coordinates pos) const
{
    return hexa::is_air_chunk(pos, w_.get_coarse_height(pos));
}

} // namespace hexa
