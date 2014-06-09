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
// Copyright 2012-2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "init_terrain_generators.hpp"

#include <string>
#include <boost/format.hpp>
#include <hexa/algorithm.hpp>
#include <hexa/trace.hpp>

#include "world.hpp"

#include "area/area_generator.hpp"
#include "area/fixed_value_area_generator.hpp"

#include "lightmap/ambient_occlusion_lightmap.hpp"
#include "lightmap/test_lightmap.hpp"
#include "lightmap/lamp_lightmap.hpp"
#include "lightmap/radiosity_lightmap.hpp"
#include "lightmap/sun_lightmap.hpp"
#include "lightmap/uniform_lightmap.hpp"

#include "terrain/cave_generator.hpp"
#include "terrain/flatworld_generator.hpp"
#include "terrain/heightmap_terrain_generator.hpp"
#include "terrain/testpattern_generator.hpp"
#include "terrain/soil_generator.hpp"

/*
#include "terrain/anvil.hpp"
#include "terrain/binvox_world_generator.hpp"
#include "terrain/gridworld_generator.hpp"
#include "terrain/ocean_generator.hpp"
#include "terrain/robinton_generator.hpp"
#include "terrain/topsurface_generator.hpp"
#include "terrain/tree_generator.hpp"
*/

using boost::format;
using namespace boost::property_tree;

namespace hexa
{

void init_terrain_gen(world& w, const ptree& config,
                      noise::generator_context& gen_ctx)
{
    auto area_def(config.get_child_optional("areas"));
    if (area_def)
        for (const auto& area : *area_def) {
            const auto& info(area.second);
            auto name(info.get<std::string>("name"));
            auto def(info.get<std::string>("def", ""));
            auto cache(info.get<std::string>("cache", ""));

            trace((format("area name '%1%'") % name).str());
            try {
                if (def.empty())
                    w.add_area_generator(
                        std::make_unique<fixed_value_area_generator>(w, info));

                else {
                    gen_ctx.set_script(name, def);
                    w.add_area_generator(
                        std::make_unique<area_generator>(w, info, gen_ctx));
                }
            } catch (ptree_bad_path& e) {
                throw std::runtime_error(
                    (format("cannot initialize area generator '%1%': required "
                            "property '%2%' missing") % name
                     % e.path<std::string>()).str());
            } catch (std::exception& e) {
                throw std::runtime_error(
                    (format("cannot initialize area generator '%1%': %2%")
                     % name % e.what()).str());
            }
        }

    auto terrain_def(config.get_child_optional("terrain"));
    if (terrain_def)
        for (const auto& def : *terrain_def) {
            const auto& info(def.second);
            auto module(info.get<std::string>("module"));

            trace((format("terrain generator %1%") % module).str());
            try {
                if (module == "caves")
                    w.add_terrain_generator(
                        std::make_unique<cave_generator>(w, info));

                else if (module == "flat")
                    w.add_terrain_generator(
                        std::make_unique<flatworld_generator>(w, info));

                else if (module == "soil")
                    w.add_terrain_generator(
                        std::make_unique<soil_generator>(w, info));

                else if (module == "heightmap_terrain")
                    w.add_terrain_generator(
                        std::make_unique<heightmap_terrain_generator>(w,
                                                                      info));

                else if (module == "testpattern")
                    w.add_terrain_generator(
                        std::make_unique<testpattern_generator>(w, info));

                /*
                            if (module == "anvil")
                                w.add_terrain_generator(std::make_unique<anvil_generator>(w,
                   info));

                            else if (module == "binvox")
                                w.add_terrain_generator(std::make_unique<binvox_world_generator>(w,
                   info));

                            else if (module == "flat")
                                w.add_terrain_generator(std::make_unique<flatworld_generator>(w,
                   info));

                            else if (module == "grid")
                                w.add_terrain_generator(std::make_unique<gridworld_generator>(w,
                   info));

                            else if (module == "robinton")
                                w.add_terrain_generator(std::make_unique<robinton_generator>(w,
                   info));

                            else if (module == "ocean")
                                w.add_terrain_generator(std::make_unique<ocean_generator>(w,
                   info));

                            else if (module == "topsurface")
                                w.add_terrain_generator(std::make_unique<topsurface_generator>(w,
                   info));

                            else if (module == "trees")
                                w.add_terrain_generator(std::make_unique<tree_generator>(w,
                   info));
                */
                else
                    throw std::runtime_error(
                        (format("unknown terrain module '%1'") % module)
                            .str());
            } catch (ptree_bad_path& e) {
                throw std::runtime_error(
                    (format("cannot initialize terrain module '%1%': required "
                            "property '%2%' missing") % module
                     % e.path<std::string>()).str());
            }
        }

    auto light_def(config.get_child_optional("light"));
    if (light_def)
        for (const auto& def : *light_def) {
            const auto& info(def.second);
            auto module(info.get<std::string>("module"));

            trace((format("lightmap %1%") % module).str());
            try {
                if (module == "ambient_occlusion")
                    w.add_lightmap_generator(
                        std::make_unique<ambient_occlusion_lightmap>(w, info));

                else if (module == "debug")
                    w.add_lightmap_generator(
                        std::make_unique<test_lightmap>(w, info));

                else if (module == "lamp")
                    w.add_lightmap_generator(
                        std::make_unique<lamp_lightmap>(w, info));

                else if (module == "radiosity")
                    w.add_lightmap_generator(
                        std::make_unique<radiosity_lightmap>(w, info));

                else if (module == "sun")
                    w.add_lightmap_generator(
                        std::make_unique<sun_lightmap>(w, info));

                else if (module == "uniform")
                    w.add_lightmap_generator(
                        std::make_unique<uniform_lightmap>(w, info));

                else
                    throw std::runtime_error(
                        (format("unknown light module '%1'") % module).str());
            } catch (ptree_bad_path& e) {
                throw std::runtime_error(
                    (format("cannot initialize light module '%1%': required "
                            "property '%2%' missing") % module
                     % e.path<std::string>()).str());
            }
        }
}

} // namespace hexa
