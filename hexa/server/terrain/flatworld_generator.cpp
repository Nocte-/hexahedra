//---------------------------------------------------------------------------
// server/terrain/flatworld_generator.cpp
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
// Copyright 2012-2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "flatworld_generator.hpp"

#include <boost/range/algorithm.hpp>
#include <hexa/area_data.hpp>
#include <hexa/block_types.hpp>

using namespace boost::range;

namespace hexa
{

flatworld_generator::flatworld_generator(
    world& w, const boost::property_tree::ptree& conf)
    : terrain_generator_i(w)
    , level_(world_chunk_center.z + (conf.get<int>("height", 0)))
    , block_type_(find_material(conf.get<std::string>("material", "stone"), 1))
{
}

void flatworld_generator::generate(world_terraingen_access&,
                                   const chunk_coordinates& pos, chunk& cnk)
{
    if (!is_air_chunk(pos, level_))
        fill(cnk, block_type_);
}

bool flatworld_generator::generate(world_terraingen_access&,
                                   const std::string& type,
                                   map_coordinates pos, area_data& data) const
{
    if (type != "surface" && type != "heightmap")
        return false;

    int16_t level(level_ * chunk_size);
    for (auto& p : data)
        p = std::max(p, level);

    return true;
}

} // namespace hexa
