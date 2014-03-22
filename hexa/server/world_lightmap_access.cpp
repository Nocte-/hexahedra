//---------------------------------------------------------------------------
// server/world_lightmap_access.hpp
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

#include "world_lightmap_access.hpp"

#include "world.hpp"

namespace hexa {

world_lightmap_access::~world_lightmap_access()
{
}

const chunk&
world_lightmap_access::get_chunk (const chunk_coordinates& pos)
{
    return w_.get_chunk(pos);
}

const surface_data&
world_lightmap_access::get_surface (const chunk_coordinates& pos)
{
    return w_.get_surface(pos);
}

} // namespace hexa

