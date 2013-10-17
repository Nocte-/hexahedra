//---------------------------------------------------------------------------
// server/init_terrain_generators.cpp
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
// Copyright 2012-2013, nocte@hippie.nu
//---------------------------------------------------------------------------
#include "init_terrain_generators.hpp"

#include <string>
#include <boost/format.hpp>
#include <hexa/algorithm.hpp>
#include <hexa/trace.hpp>

#include "world.hpp"

#include "ambient_occlusion_lightmap.hpp"
#include "test_lightmap.hpp"
#include "lamp_lightmap.hpp"
#include "radiosity_lightmap.hpp"
#include "sun_lightmap.hpp"
#include "uniform_lightmap.hpp"

#include "anvil.hpp"
#include "binvox_world_generator.hpp"
#include "biome_generator.hpp"
#include "flatworld_generator.hpp"
#include "gridworld_generator.hpp"
#include "heightmap_generator.hpp"
#include "lua_heightmap_generator.hpp"
#include "null_area_generator.hpp"
#include "ocean_generator.hpp"
#include "robinton_generator.hpp"
#include "standard_world_generator.hpp"
#include "surface_generator.hpp"
#include "soil_generator.hpp"
#include "tree_generator.hpp"

using boost::format;
using namespace boost::property_tree;

namespace hexa {

void init_terrain_gen (world& w, const ptree& config)
{
    for (const auto& area : config.get_child("areas"))
    {
        const auto& info (area.second);
        auto module (info.get<std::string>("module"));

        trace((format("area module %1%") % module).str());
        try
        {
            if (module == "standard_heightmap")
                w.add_area_generator(std::make_unique<heightmap_generator>(w, info));

            else if (module == "biome")
                w.add_area_generator(std::make_unique<biome_generator>(w, info));

            else if (module == "null")
                w.add_area_generator(std::make_unique<null_area_generator>(w, info));

            else
                std::cout << "Warning: unknown area module " << module << std::endl;
        }
        catch (ptree_bad_path& e)
        {
            throw std::runtime_error((format("cannot initialize area module '%1%': required property '%2%' missing") % module % e.path<std::string>()).str());
        }
    }

    for (const auto& area : config.get_child("terrain"))
    {
        const auto& info (area.second);
        auto module (info.get<std::string>("module"));

        trace((format("terrain generator %1%") % module).str());
        try
        {
            if (module == "anvil")
				w.add_terrain_generator(std::make_unique<anvil_generator>(w, info));

            else if (module == "binvox")
                w.add_terrain_generator(std::make_unique<binvox_world_generator>(w, info));

            else if (module == "flat")
                w.add_terrain_generator(std::make_unique<flatworld_generator>(w, info));

            else if (module == "robinton")
                w.add_terrain_generator(std::make_unique<robinton_generator>(w, info));

            else if (module == "standard")
                w.add_terrain_generator(std::make_unique<standard_world_generator>(w, info));

            else if (module == "ocean")
                w.add_terrain_generator(std::make_unique<ocean_generator>(w, info));

            else if (module == "topsurface")
                w.add_terrain_generator(std::make_unique<surface_generator>(w, info));

            else if (module == "soil")
                w.add_terrain_generator(std::make_unique<soil_generator>(w, info));

            else if (module == "trees")
                w.add_terrain_generator(std::make_unique<tree_generator>(w, info));

            else
                std::cout << "Warning: unknown terrain module " << module << std::endl;
        }
        catch (ptree_bad_path& e)
        {
            throw std::runtime_error((format("cannot initialize terrain module '%1%': required property '%2%' missing") % module % e.path<std::string>()).str());
        }
    }

    for (const auto& area : config.get_child("light"))
    {
        const auto& info (area.second);
        auto module (info.get<std::string>("module"));

        trace((format("lightmap %1%") % module).str());
        try
        {
            if (module == "ambient_occlusion")
                w.add_lightmap_generator(std::make_unique<ambient_occlusion_lightmap>(w, info));

            else if (module == "debug")
                w.add_lightmap_generator(std::make_unique<test_lightmap>(w, info));

            else if (module == "lamp")
                w.add_lightmap_generator(std::make_unique<lamp_lightmap>(w, info));

            else if (module == "radiosity")
                w.add_lightmap_generator(std::make_unique<radiosity_lightmap>(w, info));

            else if (module == "sun")
                w.add_lightmap_generator(std::make_unique<sun_lightmap>(w, info));

            else if (module == "uniform")
                w.add_lightmap_generator(std::make_unique<uniform_lightmap>(w, info));

            else
                std::cout << "Warning: unknown light module " << module << std::endl;
        }
        catch (ptree_bad_path& e)
        {
            throw std::runtime_error((format("cannot initialize light module '%1%': required property '%2%' missing") % module % e.path<std::string>()).str());
        }
    }
}

} // namespace hexa

