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

#include <cassert>
#include <hexa/voxel_range.hpp>

using namespace boost::range;

namespace hexa {

namespace {

std::vector<chunk_index> chunk_outer_shell;
std::vector<chunk_index> chunk_inner_core;

}

void
init_surface_extraction()
{
    chunk_outer_shell.reserve(512 + 448 + 392);

    // +x and -x sides
    for (int8_t y (0); y < chunk_size; ++y)
    {
        for (int8_t z (0); z < chunk_size; ++z)
        {
            chunk_outer_shell.push_back({0, y, z});
            chunk_outer_shell.push_back({chunk_size - 1, y, z});
        }
    }

    // +y and -y sides
    for (int8_t z (0); z < chunk_size; ++z)
    {
        for (int8_t x (1); x < chunk_size - 1; ++x)
        {
            chunk_outer_shell.push_back({x, 0, z});
            chunk_outer_shell.push_back({x, chunk_size - 1, z});
        }
    }

    // +z and -z sides
    for (int8_t y (1); y < chunk_size - 1; ++y)
    {
        for (int8_t x (1); x < chunk_size - 1; ++x)
        {
            chunk_outer_shell.push_back({x, y, 0});
            chunk_outer_shell.push_back({x, y, chunk_size - 1});
        }
    }
    std::sort(chunk_outer_shell.begin(), chunk_outer_shell.end());


    chunk_inner_core.reserve(2744);
    for (int8_t z (1); z < chunk_size - 1; ++z)
    {
        for (int8_t y (1); y < chunk_size - 1; ++y)
        {
            for (int8_t x (1); x < chunk_size - 1; ++x)
            {
                chunk_inner_core.push_back({x, y, z});
            }
        }
    }
}

surface
extract_opaque_surface (const world_subsection_read& terrain)
{
    assert(!chunk_outer_shell.empty());
    assert(!chunk_inner_core.empty());

    surface result;
    result.reserve(256);

    // Cache the center chunk
    const chunk& center_chunk (terrain.get_chunk({0,0,0}));

    for (chunk_index i : chunk_outer_shell)
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

    for (chunk_index i : chunk_inner_core)
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
                uint16_t other_type (center_chunk[i + dir_vector[dir]].type);

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
    assert(!chunk_outer_shell.empty());
    assert(!chunk_inner_core.empty());

    surface result;

    // Cache the center chunk
    const chunk& center_chunk (terrain.get_chunk({0,0,0}));

    for (chunk_index i : chunk_outer_shell)
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

    for (chunk_index i : chunk_inner_core)
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
            uint16_t other_type (center_chunk[i + dir_vector[dir]].type);

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

