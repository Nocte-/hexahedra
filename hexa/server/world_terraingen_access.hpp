//---------------------------------------------------------------------------
/// \file   server/world_terraingen_access.hpp
/// \brief  Access to the game world for terrain generators
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
#pragma once

#include <boost/optional.hpp>
#include "../basic_types.hpp"

namespace hexa
{

class area_data;
class chunk;
class world;

/** Access to the game world for terrain generators.
 *  The requirements of terrain generators are a little different from the
 *  other parts of the server that need write access to the world (such as
 *  Lua scripts), and so they get their own access object.  The only thing
 *  terrain generators are allowed to do is to read and write area data.
 *  They only have access to one chunk at the time (the one being generated),
 *  and cannot read or write other chunks.
 *
 *  This class can only be instanced by hexa::world.
 */
class world_terraingen_access
{
    friend class world;

protected:
    world_terraingen_access(world& w);

public:
    world_terraingen_access(const world_terraingen_access&) = delete;
    ~world_terraingen_access();

    const area_data& get_area_data(const map_coordinates& pos, uint16_t index);

    area_data& get_area_data_writable(const map_coordinates& pos,
                                      uint16_t index);

    boost::optional<const area_data&>
    try_get_area_data(const map_coordinates& pos, int index);

    boost::optional<area_data&>
    try_get_area_data_writable(const map_coordinates& pos, int index);

private:
    world& w_;
};

} // namespace hexa
