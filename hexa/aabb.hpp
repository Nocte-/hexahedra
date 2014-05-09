//---------------------------------------------------------------------------
/// \file   aabb.hpp
/// \brief  Axis-aligned bounding box.
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

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>

namespace hexa {

/** Axis-aligned bounding box.
 * Quick example:
 * @code
#include <hexa/aabb.hpp>
#include <hexa/basic_types.hpp> // For vec3i

// Create a 10 x 10 x 10 box with one corner at (2, 3, 4)
aabb<vec3i> my_box ({2, 3, 4}, 10);

assert(my_box.is_correct());
assert(width(my_box) == 10);
assert(volume(my_box) == 1000);

// Create another box that has the same shape, but shifted 2 blocks right.
auto other_box (my_box + vec3i(2, 0, 0));

// These two boxes still overlap
assert(are_overlapping(my_box, other_box));

// Clip the two boxes and print the result to stdout:
auto clipped_box (intersect(my_box, other_box));
std::cout << clipped_box << std::endl;
 * @endcode */
template <typename t>
class aabb
{
public:
    typedef typename t::value_type  coordinate_type;
    typedef t                       value_type;
    typedef aabb<t>                 self;

public:
    /** Bottom-left-front corner. */
    value_type first;

    /** Opposite corner.
     * Note that this boundary is open.  An aabb from
     * (1,2,3) to (6,7,8) would include (5,6,7), but the point
     * (5,6,8) would be outside it. */
    value_type second;

public:
    aabb () { }

    template <typename s>
    aabb (const aabb<s>& other)
        : first (other.first), second (other.second)
    { }

    /** Create an \f$1\times 1\times 1\f$ sized bounding box.
     * \param voxel  Position of the first corner. */
    aabb (value_type voxel)
        : first (voxel), second (voxel + value_type(1, 1, 1))
    { }

    /** Create an \f$n\times n\times n\f$ sized bounding box.
     * \param voxel  Position of the first corner.
     * \param size   Size of the bounding box. */
    aabb (value_type voxel, coordinate_type size)
        : first (voxel), second (voxel + value_type(size, size, size))
    { }

    /** Create a bounding box given two corners.
     * \param first_  Lower corner (closed boundary)
     * \param second_ Upper corner (open boundary) */
    aabb (value_type first_, value_type second_)
        : first (first_), second (second_)
    { }

    /** Create a bounding box given two corners. */
    aabb (coordinate_type ax, coordinate_type ay, coordinate_type az,
          coordinate_type bx, coordinate_type by, coordinate_type bz)
        : first (ax, ay, az), second (bx, by, bz)
    { }

    /** Create an initial version for determining a bounding box. */
    static aabb<t> initial()
    {
        return aabb<t>(t(std::numeric_limits<coordinate_type>::max()),
                       t(std::numeric_limits<coordinate_type>::lowest()));
    }

    /** Check whether the two corners are set up correctly.
     * For an AABB to work properly, each coordinate in the first corner
     * should be smaller than the corresponding coordinate in the second. */
    const bool is_correct() const
    {
        return first.x < second.x && first.y < second.y && first.z < second.z;
    }

    /** Swap the corners around so they are sorted properly.
     * \post is_correct() == true */
    void make_correct()
    {
        if (first.x > second.x) std::swap(first.x, second.x);
        if (first.y > second.y) std::swap(first.y, second.y);
        if (first.z > second.z) std::swap(first.z, second.z);
    }

    /** Get the width, depth, and height of the bounding box. */
    const value_type size() const
    {
        assert(is_correct());
        return second - first;
    }

    bool operator== (const self& comp) const
    {
        return first == comp.first && second == comp.second;
    }

    bool operator!= (const self& comp) const
    {
        return !operator==(comp);
    }

    self& operator+= (value_type offset)
    {
        first += offset;
        second += offset;
        return *this;
    }

    self& operator-= (value_type offset)
    {
        first -= offset;
        second -= offset;
        return *this;
    }

    self& operator*= (coordinate_type mul)
    {
        first *= mul;
        second *= mul;
        return *this;
    }

    self& operator/= (float div)
    {
        first /= div;
        second /= div;
        return *this;
    }

    self& operator++ ()
    {
        ++second;
        return *this;
    }
};

//---------------------------------------------------------------------------

template <typename t>
aabb<t> operator+ (aabb<t> lhs, t rhs)
{
    return lhs += rhs;
}

template <typename t>
aabb<t> operator+ (aabb<t> lhs, aabb<t> rhs)
{
    return aabb<t>(t(std::min(lhs.first.x,  rhs.first.x),
                     std::min(lhs.first.y,  rhs.first.y),
                     std::min(lhs.first.z,  rhs.first.z)),
                   t(std::max(lhs.second.x, rhs.second.x),
                     std::max(lhs.second.y, rhs.second.y),
                     std::max(lhs.second.z, rhs.second.z)));
}

template <typename t>
aabb<t> operator- (aabb<t> lhs, t rhs)
{
    return lhs -= rhs;
}

template <typename t, typename s>
aabb<t> operator* (aabb<t> lhs, s rhs)
{
    return lhs *= rhs;
}

template <typename t, typename s>
aabb<t> operator/ (aabb<t> lhs, s rhs)
{
    return lhs /= rhs;
}

template <typename t>
aabb<t> operator>> (const aabb<t>& lhs, int sh)
{
    return { lhs.first >> sh, ((lhs.second - t(1)) >> sh) + t(1) };
}

//---------------------------------------------------------------------------

/** Check whether two AABBs overlap. */
template <typename t>
bool are_overlapping(const aabb<t>& lhs, const aabb<t>& rhs)
{
    assert(lhs.is_correct());
    assert(rhs.is_correct());
    return    lhs.first.x < rhs.second.x && lhs.second.x > rhs.first.x
           && lhs.first.y < rhs.second.y && lhs.second.y > rhs.first.y
           && lhs.first.z < rhs.second.z && lhs.second.z > rhs.first.z;
}

/** Check whether a given voxel lies inside the AABB. */
template <class vtx>
bool is_inside(const vtx& p, const aabb<vtx>& box)
{
    assert(box.is_correct());
    return    p.x >= box.first.x && p.x < box.second.x
           && p.y >= box.first.y && p.y < box.second.y
           && p.z >= box.first.z && p.z < box.second.z;
}

/** Create a new AABB that is the intersection of two others.
 *  If lhs and rhs don't overlap anywhere, this function returns an
 *  invalid aabb.  This can be checked using is_correct(). */
template <typename t>
aabb<t> intersection(const aabb<t>& lhs, const aabb<t>& rhs)
{
    return aabb<t>(std::max(lhs.first.x,  rhs.first.x),
                   std::max(lhs.first.y,  rhs.first.y),
                   std::max(lhs.first.z,  rhs.first.z),
                   std::min(lhs.second.x, rhs.second.x),
                   std::min(lhs.second.y, rhs.second.y),
                   std::min(lhs.second.z, rhs.second.z));
}

/** Create a new AABB that bounds two other AABBs. */
template <typename t>
aabb<t> bounding_aabb(const aabb<t>& lhs, const aabb<t>& rhs)
{
    return aabb<t>(std::min(lhs.first.x,  rhs.first.x),
                   std::min(lhs.first.y,  rhs.first.y),
                   std::min(lhs.first.z,  rhs.first.z),
                   std::max(lhs.second.x, rhs.second.x),
                   std::max(lhs.second.y, rhs.second.y),
                   std::max(lhs.second.z, rhs.second.z));
}

/** Make a separating axis intersection.
 * @param stat  The static hit box
 * @param mob   The mobile hit box
 * @return The translation distances */
template <typename t>
aabb<t> sa_intersection(const aabb<t>& stat, const aabb<t>& mob)
{
    //assert(stat.is_correct());
    assert(mob.is_correct());

    return aabb<t>(stat.first - mob.second, stat.second - mob.first);
}

/** Check if the result of \a sa_intersection() returned a collision. */
template <typename t>
bool sa_intersects(const aabb<t>& sabb)
{
    return    sabb.first.x < 0 && sabb.second.x > 0
           && sabb.first.y < 0 && sabb.second.y > 0
           && sabb.first.z < 0 && sabb.second.z > 0;
}

template <typename t>
typename t::value_type sa_length (const aabb<t>& sabb, uint8_t dir)
{
    return (((dir & 1) == 0) ? sabb.second : sabb.first)[dir / 2];
}

/** Calculate the center point of a box. */
template <typename t>
typename aabb<t>::value_type center(const aabb<t>& box)
{
    return halfway(box.first, box.second);
}

/** Cast a box to a different vertex type.
 * @code
aabb<vec3i> int_box (1, 2, 3, 8, 8, 8);

auto float_box (cast_to<vec3f>(int_box));
 * @endcode */
template <typename t, typename s>
aabb<t> cast_to (const aabb<s>& box)
{
    return aabb<t>(floor(box.first), floor(box.second) + t(1,1,1));
}

/** Make a box bigger by moving one corner, and moving the opposite
 ** corner by the same amount in the other direction. */
template <typename t>
aabb<t> inflate (const aabb<t>& box, const t& amount)
{
    return aabb<t>(box.first - amount, box.second + amount);
}

/** Make a box bigger by moving one corner, and moving the opposite
 ** corner by the same amount in the other direction. */
template <typename t>
aabb<t> inflate (const aabb<t>& box, typename t::value_type amount)
{
    return inflate(box, t(amount, amount, amount));
}

/** Get the width of a box, or the size along the x axis. */
template <typename t>
typename aabb<t>::coordinate_type  width (const aabb<t>& box)
{
    return box.second.x - box.first.x;
}

/** Get the depth of a box, or the size along the y axis. */
template <typename t>
typename aabb<t>::coordinate_type  depth (const aabb<t>& box)
{
    return box.second.y - box.first.y;
}

/** Get the height of a box, or the size along the z axis. */
template <typename t>
typename aabb<t>::coordinate_type  height (const aabb<t>& box)
{
    return box.second.z - box.first.z;
}

/** Calculate the volume of a box. */
template <typename t>
typename aabb<t>::coordinate_type  volume (const aabb<t>& box)
{
    return prod(box.second - box.first);
}

} // namespace hexa

//---------------------------------------------------------------------------

namespace std {

template <typename type>
ostream& operator<< (ostream& str, const hexa::aabb<type>& box)
{
    return str << '[' << box.first << ';' << box.second << ']';
}

template <typename t> inline
string to_string (const hexa::aabb<t>& box)
{
    std::stringstream s;
    s << box;
    return s.str();
}

} // namespace std

