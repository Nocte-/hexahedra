//---------------------------------------------------------------------------
// server/area/lua_heightmap_generator.cpp
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

#include "lua_heightmap_generator.hpp"

#include <stdexcept>
#include <luabind/back_reference.hpp>

#include <hexa/block_types.hpp>

#include "../lua.hpp"

namespace hexa {

lua_heightmap_generator::lua_heightmap_generator(world& w, const boost::property_tree::ptree& conf, lua& scripting)
    : area_generator_i (w, conf)
    , lua_ (scripting)
{ }

lua_heightmap_generator::~lua_heightmap_generator()
{ }

area_data
lua_heightmap_generator::generate (map_coordinates pos)
{
    assert (pos.x < chunk_world_limit.x);
    assert (pos.y < chunk_world_limit.y);

    area_data dest;
    luabind::call_function<void>(lua_.state(), "generate_heightmap", pos.x, pos.y, &dest);
    return dest;
}

} // namespace hexa

