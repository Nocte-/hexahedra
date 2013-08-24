//---------------------------------------------------------------------------
// hexa/collision.cpp
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
// Copyright 2012-2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "collision.hpp"

#include <cassert>
#include <limits>
#include "trace.hpp"

namespace hexa {

std::pair<int, float>
calculate_impact (aabb<vector> body, vector motion,
                  const collision_aabb& block)
{
    vector impact (0, 0, 0);

    auto sa_int (sa_intersection(body, block));
    if (!sa_intersects(sa_int))
        return { -1, 0.0f };

    int shortest_axis (-1);
    float shortest (std::numeric_limits<float>::max());
    float shortest_impact (0);

    for (int axis (0); axis < 3; ++axis)
    {
        auto d (motion[axis]);
        if (d == 0)
            continue;

        // Pick the direction along this axis opposite to the motion
        int dir (axis * 2 + (motion[axis] > 0 ? 1 : 0));
        int mask (1 << dir);

        // If there's no face exposed on this side, there's no collision
        // either.
        if ((block.open_sides & mask) == 0)
            continue;

        auto axis_impact ((d < 0 ? sa_int.first : sa_int.second)[axis]);
        auto dist (axis_impact / d);

        assert(dist > 0);
        if (dist > 0 && dist < shortest)
        {
            shortest_axis = axis;
            shortest = dist;
            shortest_impact = axis_impact;
        }
    }

    return std::make_pair(shortest_axis, -shortest_impact);
}

collision_result
collide (aabb<vector> body, vector motion,
         const collision_mesh& environment, collision_result result)
{
    // Check all elements for collisions.  Every time we bump into something,
    // adjust the motion vector (sliding along the wall) and continue.  We
    // can't stop at the first collision because there's usually more than
    // one (the ground and a wall, for example).
    //
    for (;;)
    {
        auto   found (std::end(environment));
        float  longest (0.0f);
        std::pair<int, float> nearest_impact;

        for (auto env_iter (std::begin(environment));
             env_iter != std::end(environment); ++env_iter)
        {
            auto impact (calculate_impact(body, motion, *env_iter));

            if (impact.first == -1 || motion[impact.first] == 0)
            {
                continue;
            }

            auto length (impact.second / -motion[impact.first]);

            if (length > longest)
            {
                longest = length;
                found = env_iter;
                nearest_impact = impact;
            }
        }

        if (found != std::end(environment))
        {
            body.first[nearest_impact.first] += nearest_impact.second;
            body.second[nearest_impact.first] += nearest_impact.second;
            motion[nearest_impact.first] = 0;
            result.impact[nearest_impact.first] += nearest_impact.second;

            result.elements.push_back(found);
        }
        else
            return result;
    }
}

collision_result
collide (const aabb<vector>& body, vector motion,
         const collision_mesh& environment)
{
    return collide(body, motion, environment, collision_result());
}

} // namespace hexa

