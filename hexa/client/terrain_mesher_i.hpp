//---------------------------------------------------------------------------
/// \file   client/terrain_mesher_i.hpp
/// \brief  Interface for classes that make 3D meshes from terrain data
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

#include <hexa/basic_types.hpp>
#include <hexa/block_types.hpp>
#include <hexa/lightmap.hpp>

#include "vbo.hpp"

namespace hexa
{

class terrain_mesher_i
{
public:
    terrain_mesher_i(vec3i offset)
        : offset_{offset}
    {
    }

    virtual ~terrain_mesher_i() {}

    virtual void add_face(chunk_index voxel, direction_type side,
                          uint16_t texture, light intensities) = 0;

    virtual void add_custom_block(chunk_index voxel, const custom_block& model,
                                  const std::vector<light>& intensities) = 0;

    virtual gl::vbo make_buffer() const = 0;

    virtual bool empty() const = 0;

protected:
    vec3i offset_;
};

} // namespace hexa
