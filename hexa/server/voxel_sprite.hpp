//---------------------------------------------------------------------------
/// \file   server/voxel_sprite.hpp
/// \brief  A 3-D sprite that can be used to add features to the terrain.
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

#include <string>
#include <boost/multi_array.hpp>
#include <hexa/basic_types.hpp>
#include <hexa/chunk.hpp>

namespace hexa {

class world_write;

/** A voxel sprite is made of elements that combine a block type and a mask. */
struct voxel_sprite_elem : public block
{
    voxel_sprite_elem(uint16_t type = 0, bool m = false)
        : block (type)
        , mask  (m)
    { }

    /** If set to true, the sprite will replace non-air blocks. */
    bool    mask;
};

/** Small 3-D models that can be placed in the game world. */
class voxel_sprite : public boost::multi_array<voxel_sprite_elem, 3>
{
    typedef boost::multi_array<voxel_sprite_elem, 3> base;

public:
    typedef voxel_sprite_elem     value_type;

    using base::operator[];

public:
    voxel_sprite(world_coordinates size)
        : base(boost::extents[size.x][size.y][size.z])
        , offset_ (0,0,0)
    { }

    voxel_sprite(world_coordinates size, world_vector offset)
        : base(boost::extents[size.x][size.y][size.z])
        , offset_ (offset)
    { }

    world_vector offset() const { return offset_; }

    value_type operator[] (world_coordinates i) const
        {
            assert(i.x < shape()[0]);
            assert(i.y < shape()[1]);
            assert(i.z < shape()[2]);
            return (*this)[i.x][i.y][i.z];
        }

    value_type& operator[] (world_coordinates i)
        {
            assert(i.x < shape()[0]);
            assert(i.y < shape()[1]);
            assert(i.z < shape()[2]);
            return (*this)[i.x][i.y][i.z];
        }

private:
    world_vector offset_;
};

/** Paste a sprite.
 * @param w         The game world.
 * @param sprite    The sprite.
 * @param pos       Where to place it.  This is the block where the sprite's
 *                  handle will end up. */
void
paste (world_write& w, const voxel_sprite& sprite, world_coordinates pos);

/** Paste a sprite in a chunk.
 * @param cnk       A single chunk
 * @param chunk_pos The chunks's coordinates
 * @param sprite    The sprite.
 * @param pos       Where to place it.  This is the block where the sprite's
 *                  handle will end up. */
void
paste (chunk& cnk, chunk_coordinates chunk_pos, const voxel_sprite& sprite,
       world_coordinates pos);

/** Deserialize a voxel sprite (for example, from a file)
 * @param data  The serialized data
 * @return The resulting voxel sprite */
const voxel_sprite
deserialize(const std::string& data);

/** Deserialize a voxel sprite from human readable text
 * @param data  The description of the sprite
 * @return The resulting voxel sprite */
const voxel_sprite
deserialize_text (const std::string& data);

} // namespace hexa

