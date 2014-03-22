//---------------------------------------------------------------------------
/// \file   server/world_lightmap_access.hpp
/// \brief  Access to the game world for lightmap generators
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
#include "../chunk.hpp"

namespace hexa {

class area_data;
class chunk;
class surface_data;
class world;

/** Access to the game world for terrain generators.
 *  This class can only be instanced by hexa::world.
 */
class world_lightmap_access
{
    world& w_;
    friend class world;

protected:
    world_lightmap_access (world& w) : w_(w) { }

public:
    world_lightmap_access (const world_lightmap_access&) = delete;

#ifdef _MSC_VER
#else
    world_lightmap_access (world_lightmap_access&&) = default;
#endif

    ~world_lightmap_access();

    const chunk&
            get_chunk (const chunk_coordinates& pos);

    const surface_data&
            get_surface (const chunk_coordinates& pos);

    const block
            operator[] (const world_coordinates& pos)
    {
        return get_chunk(pos >> cnkshift)[pos % chunk_size];
    }
};

} // namespace hexa

