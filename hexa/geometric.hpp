//---------------------------------------------------------------------------
/// \file   geometric.hpp
/// \brief  Geometric algorithms.
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

#include <array>
#include <boost/optional.hpp>
#include "aabb.hpp"
#include "ray.hpp"
#include "vector3.hpp"

namespace hexa
{

/** Determine the intersection between a ray and an axis-aligned box.
 *  This function is based on the "Fast Ray-Box Intersection Test" by
 *  Andrew Woo, from "Graphics Gems", 1990.
 * @param r The ray
 * @param b The box to test against
 * @return The intersection point, or 'false' if they don't intersect */
template <typename T>
boost::optional<vector3<T>> ray_box_intersection(const ray<T>& r,
                                                 const aabb<vector3<T>>& b)
{
    const int dim = 3;

    typedef enum { left, right, middle } quadrant_t;

    bool inside = true;
    std::array<quadrant_t, dim> quadrant;
    std::array<T, dim> max_t;
    std::array<T, dim> candidate_plane;

    // Find the candidate planes
    for (int i = 0; i < dim; ++i) {
        if (r.origin[i] < b.first[i]) {
            quadrant[i] = left;
            candidate_plane[i] = b.first[i];
            inside = false;
        } else if (r.origin[i] > b.second[i]) {
            quadrant[i] = right;
            candidate_plane[i] = b.second[i];
            inside = false;
        } else {
            quadrant[i] = middle;
        }
    }

    // Ray starts inside the box
    if (inside)
        return r.origin;

    // Calculate distances to candidate planes.
    for (int i = 0; i < dim; ++i) {
        if (quadrant[i] != middle && r.dir[i] != 0)
            max_t[i] = (candidate_plane[i] - r.origin[i]) / r.dir[i];
        else
            max_t[i] = -1;
    }

    // Get the largest of max_t for final choice of intersection.
    int which_plane = 1;
    for (int i = 1; i < dim; ++i) {
        if (max_t[which_plane] < max_t[i])
            which_plane = i;
    }

    // Check if the final candidate is actually in the aabb.
    if (max_t[which_plane] < 0)
        return boost::optional<vector3<T>>{};

    vector result;
    for (int i = 0; i < dim; ++i) {
        if (which_plane != i) {
            result[i] = r.origin[i] + max_t[which_plane] * r.dir[i];

            if (result[i] < b.first[i] || result[i] > b.second[i])
                return boost::optional<vector3<T>>{};
        } else {
            result[i] = candidate_plane[i];
        }
    }
    return result;
}

/** Rotate a point around (0,0).
 * @param p     The point to rotate
 * @param angle The angle in radians, counter-clockwise */
template <typename Vertex>
Vertex rotate(Vertex p, double angle)
{
    double sina = std::sin(angle), cosa = std::cos(angle);
    auto x = p.x;
    p.x = p.x * cosa - p.y * sina;
    p.y = x * sina + p.y * cosa;

    return p;
}

} // namespace hexa
