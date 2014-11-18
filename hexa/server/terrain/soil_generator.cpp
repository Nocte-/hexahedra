//---------------------------------------------------------------------------
// server/terrain/soil_generator.cpp
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
// Copyright 2013-2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "soil_generator.hpp"

#include <hexa/log.hpp>
#include <hexa/trace.hpp>
#include "../hndl.hpp"
#include "../world.hpp"

using namespace boost::property_tree;

namespace hexa
{

static area_data zeroes_;

soil_generator::soil_generator(world& w, const ptree& conf)
    : terrain_generator_i(w)
    , surfacemap_{w.find_area_generator("surface")}
    , biome_func_{compile_hndl(conf.get<std::string>("hndl"))}
    , original_{find_material(conf.get<std::string>("replace", "stone"), 1)}
{
    zeroes_.clear(0);
    if (surfacemap_ < 0)
        throw std::runtime_error("soil_generator requires a surface map");

    for (auto& biome_defs : conf.get_child("material", ptree())) {
        replace_.emplace_back();
        for (auto& mat : biome_defs.second.get_child("", ptree()))
            replace_.back().emplace_back(find_material(mat.second.data()));
    }
}

void replace(int x, int y, int z, chunk& dest, uint16_t check, uint16_t type)
{
    if (z >= 0 && z < chunk_size && dest(x, y, z) == check)
        dest(x, y, z) = type;
}

void soil_generator::generate(world_terraingen_access& data,
                              const chunk_coordinates& pos, chunk& cnk)
{
    trace("soil generation for %1%", world_vector(pos - world_chunk_center));
    auto& sm = data.get_area_data(pos, surfacemap_);
    auto bm = hndl_area_int16(*biome_func_, pos);
    int16_t z_offset = convert_height_16bit(pos.z * chunk_size);

    for (uint16_t y = 0; y < chunk_size; ++y) {
        for (uint16_t x = 0; x < chunk_size; ++x) {
            auto biome_type = bm(x, y);
            if (biome_type < 0 || (uint16_t)biome_type >= replace_.size())
                continue;

            auto& column = replace_[biome_type];
            int16_t lz = sm(x, y);

            if (lz <= z_offset
                || lz - (int16_t)column.size() > z_offset + chunk_size) {
                continue;
            }
            int z = (int)lz - z_offset - 1;
            for (size_t i = 0; i < column.size(); ++i)
                replace(x, y, z - i, cnk, original_, column[i]);
        }
    }
}

} // namespace hexa
