//---------------------------------------------------------------------------
// server/ocean_generator.cpp
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
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "ocean_generator.hpp"

#include <algorithm>
#include <cassert>
#include <mutex>

#include <hexa/block_types.hpp>
#include "world.hpp"

using boost::property_tree::ptree;

namespace hexa {

ocean_generator::ocean_generator(world& w, const ptree& conf)
    : terrain_generator_i (w, conf)
    , material_(find_material(conf.get<std::string>("material", "water")))
    , level_ (water_level + conf.get<int>("level", 0))
{
    init();
    if (!material_)
    {

        throw std::runtime_error("ocean_generator requires material '"
                                 + conf.get<std::string>("material", "water")
                                 + "'");
    }
}

void ocean_generator::init()
{
    int i (w_.find_area_generator("heightmap"));
    if (i < 0)
        throw std::runtime_error("ocean_generator requires a height map");

    heightmap_ = static_cast<uint16_t>(i);
}

void ocean_generator::generate(chunk_coordinates pos, chunk& dest)
{
    uint32_t oz (pos.z * chunk_size);

    if (oz >= level_)
        return;

    area_ptr hm (w_.get_area_data(pos, heightmap_));

    int z_lim (std::min<int>(chunk_size, level_ - oz));
    assert(z_lim > 0);
    if (z_lim <= 0)
        return;

    for (int x (0); x < chunk_size; ++x)
    {
        for (int y (0); y < chunk_size; ++y)
        {
            for (int z (0); z < z_lim; ++z)
            {
                auto& blk (dest(x,y,z));
                if (blk == type::air)
                    blk = material_;
            }
        }
    }
}

} // namespace hexa

