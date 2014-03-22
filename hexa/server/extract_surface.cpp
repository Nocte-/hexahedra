//---------------------------------------------------------------------------
// hexa/server/extract_surface.cpp
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

#include "extract_surface.hpp"

#include <boost/range/algorithm.hpp>
#include <hexa/voxel_range.hpp>

using namespace boost::range;

namespace hexa {

surface
extract_opaque_surface (const world_subsection_read& terrain)
{
    surface result;
    result.reserve(256);

    // Cache the center chunk
    const chunk& center_chunk (terrain.get_chunk({0,0,0}));

    for (chunk_index i : every_block_in_chunk)
    {
        uint16_t type (center_chunk[i].type);            
        if (type == type::air)
            continue;

        if (material_prop[type].is_custom_block())
        {
            result.emplace_back(i, 0x3f, type);
        }
        else if (!type::is_transparent(type))
		{
            uint8_t dirs (0);
			for (uint8_t dir (0); dir < 6; ++dir)
			{
                uint16_t other_type (terrain[i + dir_vector[dir]].type);
            
                if (!type::is_visually_solid(other_type))
                    dirs += (1 << dir);
			}

            if (dirs != 0)
                result.emplace_back(i, dirs, type);
		}
    }

    return result;
}

surface
extract_transparent_surface (const world_subsection_read& terrain)
{
    surface result;
    result.reserve(256);

    // Cache the center chunk
    const chunk& center_chunk (terrain.get_chunk({0,0,0}));

    for (chunk_index i : every_block_in_chunk)
    {
        uint16_t type (center_chunk[i].type);
        if (type == type::air)
            continue;

        const auto& m (material_prop[type]);
        if (!m.is_transparent() || m.is_custom_block())
            continue;

        uint8_t dirs (0);
        for (uint8_t dir (0); dir < 6; ++dir)
        {
            uint16_t other_type (terrain[i + dir_vector[dir]].type);

            if (   type != other_type
                && !type::is_visually_solid(other_type)
                && m.textures[dir] != material_prop[other_type].textures[dir^1])
            {
                dirs += (1 << dir);
            }
        }

        if (dirs != 0)
            result.emplace_back(i, dirs, type);
    }

    return result;
}


} // namespace hexa

