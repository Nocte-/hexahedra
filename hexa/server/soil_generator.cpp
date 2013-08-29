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

void soil_generator::generate(chunk_coordinates pos, chunk& dest)
{
    if (!w_.is_area_data_available(pos, surfacemap_))
    {
        std::cout << "No area data available for soil" << std::endl;
        return;
    }

    auto sm  (w_.get_area_data(pos, surfacemap_));

    chunk_coordinates bot ();
    auto region (w_.lock_region({pos + world_vector(0, 0, -1), pos}, *this));
    int16_t z_offset (convert_height_16bit(pos.z * chunk_size));

    for (uint32_t ax (0); ax < chunk_size; ++ax)
    {
        for (uint32_t ay (0); ay < chunk_size; ++ay)
        {
            int16_t lz ((*sm)(ax, ay));
            if (lz < z_offset || lz >= z_offset + chunk_size)
                continue;

            uint32_t x (ax + pos.x * chunk_size);
            uint32_t y (ay + pos.y * chunk_size);
            uint32_t z (water_level + lz);
            if (region(x,y,z) == (uint16_t)16)
            {
                if (lz >= 5)
                {
                    region(x,y,z  ) = grass_;
                    region(x,y,z-1) = dirt_;
                    region(x,y,z-2) = rock_;
                }
                else
                {
                    region(x,y,z  ) = sand_;
                    region(x,y,z-1) = sand_;
                    region(x,y,z-2) = rock_;
                }
            }
        }
    }
}

} // namespace hexa

