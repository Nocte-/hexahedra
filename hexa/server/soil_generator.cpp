//---------------------------------------------------------------------------
// server/soil_generator.cpp
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
// Copyright 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "soil_generator.hpp"

#include <hexa/log.hpp>
#include <hexa/trace.hpp>
#include "world.hpp"

using namespace boost::property_tree;

namespace hexa {

soil_generator::soil_generator(world& w, const ptree& conf)
    : terrain_generator_i(w, conf)
    , surfacemap_(w.find_area_generator("surface"))
    , grass_(find_material("grass"))
    , dirt_(find_material("dirt"))
    , rock_(find_material("cobblestone"))
    , sand_(find_material("sand"))
{
    if (surfacemap_ < 0)
        throw std::runtime_error("soil_generator requires a surface map");
}

void replace (int x, int y, int z, chunk& dest, uint16_t check, uint16_t type)
{
    if (z >= 0 && z < chunk_size)
    {
        if (dest(x,y,z) == check)
            dest(x,y,z) = type;
    }
}

void soil_generator::generate(chunk_coordinates pos, chunk& dest)
{
    if (!w_.is_area_data_available(pos, surfacemap_))
    {
        log_msg("ERROR: No area data available for soil");
        return;
    }

    trace("start soil generation for %1%", world_vector(pos - world_chunk_center));

    const uint16_t old (16); // stone

    auto sm (w_.get_area_data(pos, surfacemap_));
    int16_t z_offset (convert_height_16bit(pos.z * chunk_size));

    for (int x (0); x < chunk_size; ++x)
    {
        for (int y (0); y < chunk_size; ++y)
        {
            int16_t lz ((*sm)(x, y));

            if (lz < z_offset || lz >= z_offset + chunk_size + 3)
                continue;

            int z ((int)lz - z_offset);
            if (lz >= 5)
            {
                replace(x,y,z  , dest, old, grass_);
                replace(x,y,z-1, dest, old, dirt_);
                replace(x,y,z-2, dest, old, rock_);
            }
            else
            {
                replace(x,y,z  , dest, old, sand_);
                replace(x,y,z-1, dest, old, sand_);
                replace(x,y,z-2, dest, old, rock_);
            }
        }
    }

    trace("end soil generation for %1%", world_vector(pos - world_chunk_center));
}

} // namespace hexa

