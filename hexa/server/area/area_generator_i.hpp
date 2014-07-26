//---------------------------------------------------------------------------
/// \file   server/area/area_generator_i.hpp
/// \brief  Interface for area generator modules.
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

#pragma once

#include <boost/property_tree/ptree.hpp>
#include <hexa/basic_types.hpp>
#include <hexa/area_data.hpp>

namespace hexa
{

class world;

/** Interface for area generation modules.
 *  Area generators build metadata for the 2-D map.  This could be the height
 *  map, but biomes also use this system.  If anyone wants to make a game
 *  where the world gets more dangerous the further away the player gets
 *  from the spawn point, this would be the right place to add a "danger
 *  level" to the game world.
 */
class area_generator_i
{
public:
    area_generator_i(world& w, const boost::property_tree::ptree& conf)
        : w_(w)
        , name_(conf.get<std::string>("name"))
        , cache_(conf.get<std::string>("cache", "") == "true")
    {
    }

    std::string name() const { return name_; }

    virtual ~area_generator_i() {}

    virtual area_data generate(map_coordinates) = 0;

    bool should_write_to_file() const { return cache_; }

protected:
    world& w_;
    std::string name_;
    bool cache_;
};

} // namespace hexa
