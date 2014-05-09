//---------------------------------------------------------------------------
/// \file   server/area/lua_heightmap_generator.hpp
/// \brief  Allow users to make their own height maps with Lua.
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

#pragma once

#include "area_generator_i.hpp"

namespace hexa {

class lua;

class lua_heightmap_generator : public area_generator_i
{
public:
    lua_heightmap_generator(world& w, const boost::property_tree::ptree& conf, lua& scripting);
    virtual ~lua_heightmap_generator();

    virtual area_data
    generate (map_coordinates xy) override;

private:
    lua&   lua_;
};

} // namespace hexa

