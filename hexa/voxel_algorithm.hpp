//---------------------------------------------------------------------------
/// \file   voxel_algorithm.hpp
/// \brief  Geometry algorithms for voxels.
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

#include <cmath>
#include <utility>
#include "vector3.hpp"
#include "algorithm.hpp"

namespace hexa {

inline
std::vector<vector3<int>>
voxel_raycast(vector from, vector to)
{
    std::vector<vector3<int>> result;
    result.reserve(manhattan_distance(from, to) + 1);

    // This algorithm is a cleaned up version of the one found in PolyVox.
    // http://www.volumesoffun.com/
    vector3<int> cur (floor(from)), 
                 end (floor(to)),
                 sig (signs(to - from));

    vector delta (vector(1,1,1) / absolute(to - from));
    vector min (floor(cur)), max (min + vector(1,1,1));
    vector t (((from.x > to.x) ? (from.x - min.x) : (max.x - from.x)),
              ((from.y > to.y) ? (from.y - min.y) : (max.y - from.y)),
              ((from.z > to.z) ? (from.z - min.z) : (max.z - from.z)));

    t /= absolute(to - from);

    for(;;)
    {
        result.push_back(cur);

        if (t.x <= t.y && t.x <= t.z)
        {
            if (cur.x == end.x)
                break;

            t.x += delta.x;
            cur.x += sig.x;
        }

        else if (t.y <= t.z)
        {
            if (cur.y == end.y)
                break;
            
            t.y += delta.y;
            cur.y += sig.y;
        }

        else
        {
            if (cur.z == end.z)
                break;

            t.z += delta.z;
            cur.z += sig.z;
        }
    }
    return result;
}

template <class func>
func
voxel_raycast(vector from, vector to, func op)
{
    // This algorithm is a cleaned up version of the one found in PolyVox.
    // http://www.volumesoffun.com/
    vector3<int> cur (floor(from)), 
                 end (floor(to)),
                 sig (signs(to - from));

    vector delta (vector(1,1,1) / absolute(to - from));
    vector min (floor(cur)), max (min + vector(1,1,1));
    vector t (((from.x > to.x) ? (from.x - min.x) : (max.x - from.x)),
              ((from.y > to.y) ? (from.y - min.y) : (max.y - from.y)),
              ((from.z > to.z) ? (from.z - min.z) : (max.z - from.z)));

    t /= absolute(to - from);

    for(;;)
    {
        if (op(cur))
            return op;

        if (t.x <= t.y && t.x <= t.z)
        {
            if (cur.x == end.x)
                break;

            t.x += delta.x;
            cur.x += sig.x;
        }

        else if (t.y <= t.z)
        {
            if (cur.y == end.y)
                break;
            
            t.y += delta.y;
            cur.y += sig.y;
        }

        else
        {
            if (cur.z == end.z)
                break;

            t.z += delta.z;
            cur.z += sig.z;
        }
    }
    return op;
}
inline
std::vector<vector3<int>>
dumbass_line(vector3<float> f, vector3<float> to)
{
    std::vector<vector3<int>> result;
    result.emplace_back(floor(f));

    int lim (10000 * distance(f, to));
    vector3<double> from (f);
    vector3<double> step ((to - f) / (double)lim);
    for (int i (0); i < lim; ++i)
    {
        from += step;
        vector3<int> a (floor(from));
        if (a != result.back())
            result.push_back(a);
    }

    return result;
}

inline
std::vector<vector3<int>>
dumbass_line(vector3<int> from, vector3<int> to)
{
    vector3<float> half (0.5f, 0.5f, 0.5f);
    return dumbass_line(vector3<float>(from) + half, vector3<float>(to) + half);
}

} // namespace hexa

