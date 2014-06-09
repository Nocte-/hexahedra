//---------------------------------------------------------------------------
/// \file   plane3d.hpp
/// \brief  An infinite plane in 3-D.
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

#include "vector3.hpp"

namespace hexa
{

/** An infinite plane in 3-D space */
template <class type>
class plane3d
{
public:
    /** The plane's normal vector. */
    vector3<type> normal;
    /** The distance to origin. */
    type distance;

public:
    plane3d() {}

    /** Construct a plane that intersects the three given points. */
    plane3d(const vector3<type>& a, const vector3<type>& b,
            const vector3<type>& c)
        : normal(normalize(cross_product(a - b, c - b)))
        , distance(-dot_prod(b, normal))
    {
    }

    /** Construct a plane with a given normal that intersects a point. */
    plane3d(const vector3<type>& normal_, const vector3<type>& a)
        : normal(normal_)
        , distance(-dot_prod(a, normal))
    {
    }

    /** Flip the plane around, so it faces the other way. */
    void flip()
    {
        normal = -normal;
        distance = -distance;
    }
};

/** Calculate the shortest distance from a point to a plane. */
template <typename type>
type distance(const plane3d<type>& plane, const vector3<type>& point)
{
    return dot_prod(point, plane.normal) + plane.distance;
}

} // namespace hexa
