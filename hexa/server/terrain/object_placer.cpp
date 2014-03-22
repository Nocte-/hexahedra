//---------------------------------------------------------------------------
// server/terrain/object_placer.cpp
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

#include "object_placer.hpp"

#include <hexa/algorithm.hpp>
#include "../random.hpp"
#include "../world.hpp"

using namespace boost::property_tree;

namespace hexa {

object_placer::object_placer(world& w, const ptree& conf)
    : terrain_generator_i(w)
    , density_map_(w.find_area_generator(conf.get<std::string>("density_map")))
{
    if (density_map_ < 0)
        throw std::runtime_error("object_placer: cannot find 'density_map'");

    auto& list (conf.get_child("sprites", ptree()));
    for (auto& elem : list)
        sprites_.emplace_back(deserialize(file_contents(elem.second.data())));
}

void
object_placer::generate(world_terraingen_access& data,
                        const chunk_coordinates& pos,
                        chunk& cnk)
{

}

chunk_height
object_placer::estimate_height (world_terraingen_access& data,
                                map_coordinates xy,
                                chunk_height prev) const
{
    return prev;
}

} // namespace hexa

