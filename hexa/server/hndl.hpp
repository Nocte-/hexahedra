//---------------------------------------------------------------------------
/// \file   server/hndl.hpp
/// \brief  Compile HNDL scripts
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
// Copyright (C) 2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include <memory>
#include <hexanoise/generator_i.hpp>
#include <hexa/array.hpp>
#include <hexa/basic_types.hpp>

namespace hexa
{

void init_hndl();

std::unique_ptr<noise::generator_i> compile_hndl(const std::string& script);

std::unique_ptr<noise::generator_i> compile_hndl(const std::string& name,
                                                 const std::string& script);

inline array_2d<int16_t, chunk_size, chunk_size>
hndl_area_int16(noise::generator_i& gen, map_coordinates pos)
{
    map_rel_coordinates p = pos - map_chunk_center;
    return gen.run_int16(glm::dvec2{p.x, p.y} * (double)chunk_size,
                         glm::dvec2{1, 1}, 
                         glm::ivec2{chunk_size, chunk_size});
}

inline array_2d<double, chunk_size, chunk_size>
hndl_area_double(noise::generator_i& gen, map_coordinates pos)
{
    map_rel_coordinates p = pos - map_chunk_center;
    return gen.run(glm::dvec2{p.x, p.y} * (double)chunk_size,
                   glm::dvec2{1, 1}, 
                   glm::ivec2{chunk_size, chunk_size});
}

inline array_3d<double, chunk_size, chunk_size, chunk_size>
hndl_chunk(noise::generator_i& gen, chunk_coordinates pos)
{
    world_rel_coordinates p = pos - world_chunk_center;
    return gen.run(glm::dvec3(p.x, p.y, p.z) * (double)chunk_size,
                   glm::dvec3(1, 1, 1),
                   glm::ivec3(chunk_size, chunk_size, chunk_size));
}

void set_global_variable (const std::string& name, double val);

} // namespace hexa
