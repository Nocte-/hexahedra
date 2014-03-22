//---------------------------------------------------------------------------
// server/voxel_sprite.cpp
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

#include "voxel_sprite.hpp"

#include <cassert>
#include <boost/range/algorithm.hpp>
#include <hexa/aabb.hpp>
#include <hexa/serialize.hpp>
#include <hexa/voxel_range.hpp>
#include "world.hpp"

using namespace boost::range;

namespace hexa {

void paste (world_write& w, const voxel_sprite& sprite, world_coordinates pos)
{
    world_coordinates corner (pos - sprite.offset());
    auto s (sprite.shape());
    world_coordinates size (s[0], s[1], s[2]);

    // The bounding box of the sprite as it will appear in the game world.
    aabb<world_coordinates> sprite_box (corner, corner + size);

    for_each(to_chunk_range(sprite_box), [&](chunk_coordinates c)
    {
        auto& terrain (w.get_chunk(c));
        paste (terrain, c, sprite, pos);
    });
}

void paste (chunk& cnk, chunk_coordinates cnk_pos,
            const voxel_sprite& sprite, world_coordinates pos)
{
    world_coordinates corner (pos - sprite.offset());
    auto s (sprite.shape());
    world_coordinates size (s[0], s[1], s[2]);

    // The bounding box of the sprite as it will appear in the game world.
    aabb<world_coordinates> sprite_box (corner, corner + size);

    // The bounding box of the chunk we're drawing the sprite in.
    aabb<world_coordinates> cnk_box (cnk_pos);
    cnk_box *= chunk_size;

    // The intersection between both bounding boxes is the
    // range of voxels we need to iterate over.
    auto inter (intersection(sprite_box, cnk_box));

    // Abort if the intersection is empty.
    if (!inter.is_correct())
        return;

    for_each(range<world_coordinates>(inter), [&](world_coordinates w)
    {
        auto sprite_voxel   (sprite[w - corner]);
        auto& terrain_voxel (cnk[w % chunk_size]);

        // Solid blocks only get overwritten by the sprite if
        // the mask flag is set.
        if (terrain_voxel == type::air || sprite_voxel.mask)
        {
            terrain_voxel = sprite_voxel;
        }
    });
}

const voxel_sprite
deserialize (const std::string& buf)
{
    auto ds (make_deserializer(buf));

    std::vector<std::string> block_types;
    vector3<uint32_t> dimensions;
    vector3<int32_t>  offset;

    ds(block_types)(dimensions)(offset);

    std::vector<uint16_t> tx (block_types.size());

    int i (0);
    for (auto name : block_types)
        tx[i++] = find_material(name);

    voxel_sprite result (dimensions, offset);
    voxel_sprite::value_type* ptr (result.data());
    uint8_t type;

    for (size_t i (0); i < prod(dimensions); ++i, ++ptr)
    {
        ds(type)(ptr->mask);
        if (type < tx.size())
            ptr->type = tx[type];
        else
            ptr->type = type;
    }

    return result;
}

const voxel_sprite
deserialize_text(const std::string& buf)
{
    std::stringstream str (buf);

    unsigned int num_mat;
    str >> num_mat;

    std::vector<std::string> block_types (num_mat);

    for (unsigned int i (0); i < num_mat ; ++i)
        str >> block_types[i];

    vector3<uint32_t> dimensions;
    vector3<int32_t>  offset;

    str >> dimensions.x >> dimensions.y >> dimensions.z;
    str >> offset.x >> offset.y >> offset.z;

    std::vector<uint16_t>    tx (num_mat);
    int i (0);
    for (auto name : block_types)
        tx[i++] = find_material(name);

    voxel_sprite result (dimensions, offset);
    for (size_t z (0); z < dimensions[2]; ++z)
    for (size_t y (0); y < dimensions[1]; ++y)
    for (size_t x (0); x < dimensions[0]; ++x)
    {
        uint16_t type;
        str >> type;

        if (type < tx.size())
            result[x][y][z].type = tx[type];
        else
            result[x][y][z].type = type;
    }

    return result;
}

} // namespace hexa

