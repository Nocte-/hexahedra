//---------------------------------------------------------------------------
/// \file   basic_types.hpp
/// \brief  Commonly used data types and constants.
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

#include <array>
#include <cstdint>

#include "compiler_fix.hpp"
#include "vector2.hpp"
#include "vector3.hpp"

namespace hexa
{

/** The size of a \ref hexa::chunk "chunk" (that is, the length of one of
 ** the edges). */
constexpr uint16_t chunk_size = 16;

/** Arithmetic shift for converting between chunk and world coordinates. */
constexpr uint16_t cnkshift = 4; // == std::log2(chunk_size)

/** The area of a side of a \ref hexa::chunk "chunk". */
constexpr uint16_t chunk_area = chunk_size * chunk_size;

/** The volume of a \ref hexa::chunk "chunk". */
constexpr uint16_t chunk_volume = chunk_area * chunk_size;

/** The dimensions of a chunk. */
const vector3<uint16_t> chunk_dimensions = {16, 16, 16};

/** The position of a \ref hexa::block "block" in the game world. */
typedef vector3<uint32_t> world_coordinates;

/** Relative coordinates; used sometimes when translating from other formats.
 */
typedef vector3<int32_t> world_rel_coordinates;

/** A position within a voxel, in 1/256 units.
 *  This one is used sometimes instead of a floating point representation,
 *  when compactness is more important than precision. */
typedef vector3<uint8_t> fraction_coordinates;

/** The position of a \ref hexa::chunk "chunk" in the game world.
 *  Note that unlike world_coordinates, this one has a limit.
 * \sa chunk_world_limit */
typedef vector3<uint32_t> chunk_coordinates;

/** The limits of a \ref hexa::chunk_coordinates "chunk_coordinates"
 ** position (the world size divided by the chunk size). */
const chunk_coordinates chunk_world_limit
    = chunk_coordinates(268435456, 268435456, 268435456);

/** The limits of a block's coordinates inside a chunk. */
const chunk_coordinates block_chunk_limit
    = chunk_coordinates(chunk_size, chunk_size, chunk_size);

const vector3<uint8_t> block_chunk_size
    = vector3<uint8_t>(chunk_size, chunk_size, chunk_size);

/** A world coordinate, without a height value. */
typedef vector2<uint32_t> map_world_coordinates;

/** A chunk coordinate, without a height value. */
typedef vector2<uint32_t> map_coordinates;

/** A relative chunk coordinate, without a height value. */
typedef vector2<int32_t> map_rel_coordinates;

/** The position of a \ref hexa::block "block" inside a
 *  \ref hexa::chunk "chunk".
 *  Usually the ordinates are in the range [0..chunk_size>, but in some cases
 *  it can go outside these limits, and even get negative.  (For example
 *  when using the \a hexa::neighborhood class.) */
typedef vector3<int8_t> chunk_index;

/** A coordinate within a chunk, without the height value. */
typedef vector2<uint8_t> map_index;

/** A floating point 3-D vector. */
typedef vector3<float> vector;

/** An integer vector in a chunk. */
typedef vector3<int8_t> block_vector;

/** An integer vector in the game world. */
typedef vector3<int32_t> world_vector;

/** Yaw and pitch angles, in radians.
 *  Yaw is positive clockwise from north. Pitch is down from zenith. */
typedef vector2<float> yaw_pitch;

typedef vector2<float> vec2f;
typedef vector2<int8_t> vec2b;
typedef vector2<int16_t> vec2s;
typedef vector2<int32_t> vec2i;
typedef vector3<float> vec3f;
typedef vector3<int8_t> vec3b;
typedef vector3<int16_t> vec3s;
typedef vector3<int32_t> vec3i;

//---------------------------------------------------------------------------

/** The six cardinal directions. */
enum direction_type : uint8_t {
    dir_east = 0,
    dir_west = 1,
    dir_north = 2,
    dir_south = 3,
    dir_up = 4,
    dir_down = 5
};

/** Unit vectors in the \ref hexa::direction_type "six directions". */
extern const block_vector dir_vector[6];

/** A block at (0,0,0), and its six neighbors. */
extern const block_vector neumann_neighborhood[7];

/** Data type used for the coarse heightmap.
 *  Note that this height does not span the usual 2^32 range; it is measured
 *  in chunks, so the max value is 2^28. */
typedef uint32_t chunk_height;

/** Magic value to mark an undefined part of the coarse heightmap. */
constexpr chunk_height undefined_height = 0xffffffff;

/** The center of the world, in block coordinates. */
const world_coordinates world_center = world_coordinates(2147483648u);

/** The center of the world, in chunk coordinates. */
const chunk_coordinates world_chunk_center = world_center / chunk_size;

/** The center of the world, in map coordinates. */
const map_coordinates map_chunk_center
    = map_coordinates(world_chunk_center.x, world_chunk_center.y);

/** The game uses a global clock in 10ms ticks. */
typedef uint32_t gameclock_t;

/** Container type for raw binary data. */
typedef std::vector<uint8_t> binary_data;

//---------------------------------------------------------------------------

/** Check if a chunk position is within the legal range (0..2^28-1). */
inline bool is_legal_chunk_coordinate(const chunk_coordinates& p)
{
    return p.x < chunk_world_limit.x && p.y < chunk_world_limit.y
           && p.z < chunk_world_limit.z;
}

/** Check if a chunk consists only of air blocks, based on the z-value taken
 ** from the coarse height map. */
inline bool is_air_chunk(const chunk_coordinates& p, chunk_height h)
{
    return h != undefined_height && p.z >= h;
}

/** Check if the coarse height map needs to be adjusted, based on a chunk
 ** that does not consist entirely of air, and the current height in the
 ** map. */
inline bool needs_chunk_height_adjustment(const chunk_coordinates& p,
                                          chunk_height h)
{
    return h == undefined_height || p.z >= h;
}

/** Return a new height value for the coarse height map, based on its current
 ** value, and the position of a chunk that has non-air blocks in it. */
inline chunk_height adjust_chunk_height(const chunk_coordinates& p,
                                        chunk_height h)
{
    return needs_chunk_height_adjustment(p, h) ? p.z + 1 : h;
}

/** Convert a normal height to a 16-bit height.
 *  This form is used by the terrain generation, which is limited to 65,000
 *  vertical range. */
inline int16_t convert_height_16bit(uint32_t h)
{
    return static_cast<int16_t>(static_cast<int32_t>(h - world_center.z));
}

} // namespace hexa
