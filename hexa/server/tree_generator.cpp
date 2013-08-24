//---------------------------------------------------------------------------
// server/tree_generator.cpp
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

#include "tree_generator.hpp"

#include "world.hpp"

using namespace boost::property_tree;

namespace hexa {

tree_generator::tree_generator(world& w, const ptree& conf)
    : terrain_generator_i(w, conf)
    , surfacemap_(w.find_area_generator("surface"))
    , wood_(find_material("wood (oak)"))
    , leaves_(find_material("leaves (oak)"))
    , dirt_(find_material("grass"))
{
    if (surfacemap_ < 0)
        throw std::runtime_error("tree_generator requires a surface map");
}

void tree_generator::generate(chunk_coordinates pos, chunk& dest)
{
    if (!w_.is_area_data_available(pos, surfacemap_))
        return;

    auto sm  (w_.get_area_data(pos, surfacemap_));
    int16_t z_offset (convert_height_16bit(pos.z * chunk_size));

    range<chunk_coordinates> rng (pos - chunk_coordinates(1, 1, 0),
                                  pos + chunk_coordinates(2, 2, 2));
    auto region (w_.lock_range(rng, *this));

    for (int count (0) ; count < 1; ++count)
    {
        uint8_t tx (pos.x * chunk_size + (rand() % chunk_size));
        uint8_t ty (pos.y * chunk_size + (rand() % chunk_size));

        int16_t zpos ((*sm)(tx, ty));
        if (zpos < z_offset || zpos > z_offset + chunk_size - 1)
            return;

        uint32_t z (water_level + zpos);
        if (region(tx, ty, z) != dirt_)
            continue;

        for (int c (0); c < 9; ++c)
        {
            if (c > 3)
            {
                for (int x (-3); x <= 3; ++x)
                 for (int y (-3); y <= 3; ++y)
                     if (rand() % 100 > 30)
                       region(tx+x, ty+y, z) = leaves_;
            }
            region(tx, ty, z) = wood_;
            ++z;
        }
        for (int x (-1); x <= 1; ++x)
           for (int y (-1); y <= 1; ++y)
               region(tx+x, ty+y, z) = leaves_;
    }
}

} // namespace hexa

