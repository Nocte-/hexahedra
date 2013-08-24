//---------------------------------------------------------------------------
/// \file   block_types.hpp
/// \brief  Block types.
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
// Copyright 2012, 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <boost/range/algorithm.hpp>
#include "aabb.hpp"
#include "basic_types.hpp"

namespace hexa {

/** Definition of a custom block shape. */
struct custom_block_part
{
    /** Every box is axis-aligned, and has integer coordinates ranging from
     ** 0 to 16. */
    aabb<chunk_index>       box;

    /** Every face of the box can be textured individually. */
    std::array<uint16_t, 6> textures;

    template <class archive>
    archive& serialize(archive& ar)
    {
        ar(box.first)(box.second);
        for (int i (0); i < 6; ++i)
            ar(textures[i]);

        return ar;
    }
};

/** Custom block models are defined by one or more textured boxes. */
typedef std::vector<custom_block_part> custom_block;

/** Definition of a material type. */
struct material
{
    /** Textures for the 6 block faces. */
    std::array<uint16_t, 6> textures;
    /** Material strength. */
    uint16_t                strength;
    /** How much light can pass through. */
    uint8_t                 transparency;
    /** How much light this material emits. */
    uint8_t                 light_emission;
    /** Whether this material is solid (used in collision checks). */
    bool                    is_solid;
    /** Human-readable name. */
    std::string             name;
    /** 3-D model for fancy custom blocks. */
    custom_block            model;


    material()
        : strength(16)
        , transparency(0)
        , light_emission(0)
        , is_solid(true)
        , name()
    {
        boost::range::fill(textures, 0);
    }

    bool operator== (const std::string& compare) const
        { return compare == name; }

    bool is_transparent() const
        { return transparency > 0; }

    bool is_custom_block() const
        { return !model.empty(); }

    bool is_visually_solid() const
        { return !is_transparent() && !is_custom_block(); }

    /** Serialize to an archive. */
    template <class archive>
    archive& serialize(archive& ar)
    {
        for (int i (0); i < 6; ++i)
            ar(textures[i]);

        return ar(strength)(transparency)(light_emission)(is_solid)
                 (name)(model);
    }
};

//---------------------------------------------------------------------------

// lol global variables. u mad?

/** Array of material properties. */
extern std::vector<material>                            material_prop;

/** Mapping texture names to their index. */
extern std::unordered_map<std::string, uint16_t>        texture_names;

//---------------------------------------------------------------------------

/** Register a new material by ID and return its record. */
material& register_new_material (uint16_t type_id);

/** Search for a material ID by name. */
uint16_t  find_material (const std::string& name);

//---------------------------------------------------------------------------

namespace type {

constexpr uint16_t air = 0;

/** Check if a block type is transparent. */
inline bool
is_visually_solid (uint16_t type)
{
    return material_prop[type].is_visually_solid();
}

/** Check if a block type is transparent. */
inline bool
is_transparent (uint16_t type)
{
    return material_prop[type].is_transparent();
}

/** Check if a block type is solid. */
inline bool
is_solid (uint16_t type)
{
    return material_prop[type].is_solid;
}

}} // namespace hexa::type
