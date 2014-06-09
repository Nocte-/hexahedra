//---------------------------------------------------------------------------
/// \file   ray.hpp
/// \brief  A line starting from a point and extending infinitely in one
/// direction
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
// Copyright 2013-2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include "basic_types.hpp"
#include "vector3.hpp"

namespace hexa
{

/** Infinite ray. */
template <class t>
class ray
{
    typedef ray<t> self;

public:
    typedef t ordinate_type;

public:
    vector3<t> origin; /**< The ray leaves from this point. */
    vector3<t> dir;    /**< The ray's direction. */

public:
    ray(vector3<t> o, vector3<t> d)
        : origin{o}
        , dir{d}
    {
    }

    /** Construct a ray from an origin point and spherical coordinates.
     * @param o The ray's origin
     * @param angle The ray's direction given as a yaw and pitch */
    ray(vector3<t> o, const yaw_pitch& angle)
        : origin{o}
        , dir{from_spherical<ordinate_type>(angle)}
    {
    }

    bool is_valid() const { return dir != vector3<t>(0); }

    self& normalize()
    {
        dir = normalize(dir);
        return *this;
    }

    vector3<t> at(ordinate_type distance) const
    {
        return origin + dir * distance;
    }

    self& operator+=(const vector3<t>& offset)
    {
        origin += offset;
        return *this;
    }

    self& operator-=(const vector3<t>& offset)
    {
        origin -= offset;
        return *this;
    }
};

//---------------------------------------------------------------------------

template <class t>
ray<t> operator+(ray<t> r, const vector3<t>& offset)
{
    return r += offset;
}

template <class t>
ray<t> operator-(ray<t> r, const vector3<t>& offset)
{
    return r -= offset;
}

template <class t>
ray<t> normalized(ray<t> r)
{
    return r.normalize();
}

} // namespace hexa
