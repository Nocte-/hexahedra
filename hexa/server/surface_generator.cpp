//---------------------------------------------------------------------------
// server/surface_generator.cpp
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

#include "surface_generator.hpp"

#include "world.hpp"

using namespace boost::property_tree;

namespace hexa {

surface_generator::surface_generator(world& w, const ptree& conf)
    : terrain_generator_i(w, conf)
    , heightmap_(w.find_area_generator("heightmap"))
    , surfacemap_(w.find_area_generator("surface"))
{
    if (heightmap_ < 0)
        throw std::runtime_error("surface_generator requires a height map");

    if (surfacemap_ < 0)
        throw std::runtime_error("surface_generator requires a surface map");
}

void surface_generator::generate(chunk_coordinates pos, chunk& dest)
{
    constexpr int search_limit (32);

    if (w_.is_area_data_available(pos, surfacemap_))
        return;

    area_ptr sa (std::make_shared<area_data>());
    constexpr int16_t undefined (std::numeric_limits<uint16_t>::max());
    std::fill(sa->begin(), sa->end(), undefined);

    auto top (w_.get_coarse_height(pos));
    unsigned int count (0);
    for (chunk_height chunk_z (top); chunk_z > top - search_limit; --chunk_z)
    {
        chunk_coordinates cc (pos.x, pos.y, chunk_z);
        auto locked_region (w_.lock_region({cc}, *this));
        auto chunk (w_.get_raw_chunk(cc));

        if (chunk == nullptr)
            continue;

        for (uint16_t x (0); x < chunk_size ; ++x)
        {
            for (uint16_t y (0); y < chunk_size; ++y)
            {
                if ((*sa)(x,y) != undefined)
                    continue;

                for (int z (chunk_size - 1); z >= 0; --z)
                {
                    if ((*chunk)(x,y,z) != type::air)
                    {
                        (*sa)(x,y) = convert_height_16bit(chunk_z * chunk_size + z);
                        ++count;
                        break;
                    }
                }
                if (count == sa->size())
                    goto done; // Bite me, Dijkstra.
            }
        }
    }

    done:
    if (count == 0)
    {
        std::cout << "No surface found for " << pos << std::endl;
    }
    else if (count == sa->size())
    {
        w_.store(map_coordinates(pos), surfacemap_, sa);
    }
    else
    {
        std::cout << "Incomplete " << pos << " :" << count << std::endl;
    }
}

} // namespace hexa

