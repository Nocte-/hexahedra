//---------------------------------------------------------------------------
/// \file   hexa/collision.hpp
/// \brief  Collision detection and handling.
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
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <numeric>
#include <vector>
#include <boost/range/algorithm/for_each.hpp>

#include "aabb.hpp"
#include "basic_types.hpp"
#include "vector2.hpp"
#include "vector3.hpp"

namespace hexa {

/** An axis-aligned bounding box used in collision detection.
 * The main difference with a normal AABB is that this one also keeps
 * track of open and solid faces. */
class collision_aabb : public aabb<vector>
{
public:
    /** A bitmask that marks which sides of the box are not solid. */
    uint8_t     open_sides;

    collision_aabb(aabb<vector> box, uint8_t directions)
        : aabb       (box)
        , open_sides (directions)
    { }

    vector minimum_translation_vector(const aabb<vector>& mob) const
    {
        vector result (0,0,0);

        if (!sa_intersects(mob))
            return result;

        float shortest (std::numeric_limits<float>::max());

        if (open_sides & 1) // +x
        {
            shortest = mob.second.x;
            result = vector(mob.second.x, 0, 0);
        }
        if ((open_sides & 2) && -mob.first.x < shortest) // -x
        {
            shortest = -mob.first.x;
            result = vector(mob.first.x, 0, 0);
        }

        if ((open_sides & 4) && mob.second.y < shortest) // +y
        {
            shortest = mob.second.y;
            result = vector(0, mob.second.y, 0);
        }
        if ((open_sides & 8) && -mob.first.y < shortest) // -y
        {
            shortest = -mob.first.y;
            result = vector(0, mob.first.y, 0);
        }

        if ((open_sides & 16) && mob.second.z < shortest) // +z
        {
            shortest = mob.second.z;
            result = vector(0, 0, mob.second.z);
        }
        if ((open_sides & 32) && -mob.first.z < shortest) // -z
        {
            result = vector(0, 0, mob.first.z);
        }

        return result;
    }
};

/** List all collisions between a box and the environment.
 * The result will be the set of overlapping boxes.
 * \param body  The body to check against the environment.
 * \param environment  A set of boxes
 * \result A set of boxes that are both inside \a body and \a environment */
template <class type>
std::vector<aabb<type>>
collisions(aabb<type> body, const std::vector<aabb<type>>& environment)
{
    std::vector<aabb<type>> result;
    for (auto& elem : environment)
    {
        aabb<type> hit (sat_intersection(body, elem));
        if (sat_intersects(hit))
            result.emplace_back(elem);
    }
    return result;
}

typedef std::vector<collision_aabb> collision_mesh;

/** A collision check will return an impact vector, and a list of the
 * elements that were hit. */
struct collision_result
{
    /** The difference between the input motion, and the
     ** actual motion after the collisions have been handled. */
    vector impact;

    collision_result() : impact(0, 0, 0) { }

    /** A list of all elements the body collided with. */
    std::list<collision_mesh::const_iterator> elements;
};

/** Check a moving body against a collision mesh. */
collision_result
collide (const aabb<vector>& body, vector motion,
         const collision_mesh& environment);

/** Calculate the impact of a moving body with a static aabb. */
std::pair<int, float>
calculate_impact(aabb<vector> body, vector motion,
                 const collision_aabb& block);

} // namespace hexa

