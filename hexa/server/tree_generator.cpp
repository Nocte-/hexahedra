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

#include <hexa/log.hpp>
#include <hexa/trace.hpp>
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
    log_msg("ERROR: tree_generator called with single chunk");
}

void tree_generator::generate(chunk_coordinates pos,
                              world_subsection<chunk_ptr>& region)
{
    trace("start tree generation for %1%", world_vector(pos - world_chunk_center));
    auto sm  (w_.get_area_data(pos, surfacemap_));

    if (sm == nullptr)
    {
        trace("ERROR: can't plant trees at %1%, no surface data", world_vector(pos - world_chunk_center));
        return;
    }

    int16_t z_offset (convert_height_16bit(pos.z * chunk_size));
    for (int count (0) ; count < 1; ++count)
    {
        uint8_t ox (rand() % chunk_size);
        uint8_t oy (rand() % chunk_size);

        int16_t zpos ((*sm)(ox, oy));

        trace("About to plant at %1% %2% (%3%)", (int)ox, (int)oy, world_vector(pos - world_chunk_center));

        if (zpos < z_offset || zpos > z_offset + chunk_size - 1)
        {
            trace("not in range %1% %2%", zpos, z_offset);
            continue;
        }

        uint32_t x (pos.x * chunk_size + ox);
        uint32_t y (pos.y * chunk_size + oy);
        uint32_t z (water_level + zpos);

        if (region(x, y, z) != dirt_)
        {
            trace("region %1% is not dirt", world_coordinates(x,y,z));
            continue;
        }

        trace("planting a tree");
        for (int c (0); c < 9; ++c)
        {
            if (c > 3)
            {
                for (int sx (-3); sx <= 3; ++sx)
                 for (int sy (-3); sy <= 3; ++sy)
                     if (rand() % 100 > 30)
                       region(x+sx, y+sy, z) = leaves_;
            }
            region(x, y, z) = wood_;
            ++z;
        }
        for (int sx (-1); sx <= 1; ++sx)
           for (int sy (-1); sy <= 1; ++sy)
               region(x+sx, y+sy, z) = leaves_;
    }
    trace("end tree generation for %1%", world_vector(pos - world_chunk_center));
}

chunk_height
tree_generator::estimate_height (map_coordinates xy, chunk_height prev) const
{
    if (prev != undefined_height)
        return prev + 1;

    return prev;
}

std::set<world_vector> tree_generator::span() const
{
    auto r (make_range<world_vector>( {-1, -1, 0 }, { 2, 2, 2 }));
    return { r.begin(), r.end() };
}

} // namespace hexa

