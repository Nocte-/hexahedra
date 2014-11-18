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

#include <boost/filesystem.hpp>

#include <hexa/algorithm.hpp>
#include "../hndl.hpp"
#include "../random.hpp"
#include "../voxel_sprite.hpp"
#include "../world.hpp"

namespace fs = boost::filesystem;
using namespace boost::property_tree;

namespace hexa
{

object_placer::object_placer(world& w, const ptree& conf)
    : terrain_generator_i{w}
    , density_func_{compile_hndl(conf.get<std::string>("hndl"))}
    , surface_map_{w.find_area_generator(
          conf.get<std::string>("surface_map", "surface"))}
{
    if (surface_map_ < 0)
        throw std::runtime_error("object_placer: cannot find 'surface_map'");

    auto& list = conf.get_child("sprites", ptree{});
    for (auto& elem : list) {
        fs::path filename{elem.second.data()};
        std::string content{file_contents(filename)};

        if (filename.extension() == ".txt")
            sprites_.emplace_back(deserialize_text(content));
        else
            sprites_.emplace_back(deserialize(content));
    }

    if (sprites_.empty())
        throw std::runtime_error("object_placer: no voxel sprites defined");
}

void object_placer::generate(world_terraingen_access& data,
                             const chunk_coordinates& pos, chunk& cnk)
{
    map_coordinates mpos{pos};
    for (int y = -1; y < 2; ++y) {
        for (int x = -1; x < 2; ++x) {
            map_rel_coordinates o(x, y);
            auto placement = spots(mpos + o);
            auto& surface = data.get_area_data(mpos + o, surface_map_);

            for (auto& p : placement) {
                paste(
                    cnk, pos, sprites_[0],
                    world_coordinates{p.x, p.y, world_center.z
                                                + surface(p.x % chunk_size,
                                                          p.y % chunk_size)});
            }
        }
    }
}

chunk_height object_placer::estimate_height(world_terraingen_access& data,
                                            map_coordinates xy,
                                            chunk_height prev) const
{
    return prev + 1; ///\todo Figure out a correct way to do this
}

std::vector<map_world_coordinates>
object_placer::spots(const map_coordinates& pos)
{
    std::vector<map_world_coordinates> result;
    auto distrib = hndl_area_double(*density_func_, pos);
    uint32_t rng = fnv_hash(pos);
    auto offset = pos * chunk_size;

    for (uint16_t y = 0; y < chunk_size; ++y) {
        for (uint16_t x = 0; x < chunk_size; ++x) {
            if (prng_next_zto(rng) < distrib(x, y))
                result.emplace_back(offset.x + x, offset.y + y);
        }
    }
    return result;
}

} // namespace hexa
