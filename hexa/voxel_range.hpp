//---------------------------------------------------------------------------
/// \file   voxel_range.hpp
/// \brief  Iterate over all voxels in an axis-aligned box
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

#include <cassert>
#include <iterator>
#include <iostream>

#include "aabb.hpp"
#include "basic_types.hpp"

namespace hexa
{

/** Provides iteration over an axis-aligned box between two points. */
template <class t>
class range
{
public:
    /** Iterator inside a range */
    class iterator
    {
        const range& range_;
        t cursor_;

    public:
        typedef t value_type;
        typedef t reference;
        typedef t* pointer;
        typedef size_t difference_type;

        typedef std::forward_iterator_tag iterator_category;

    public:
        /** Construct a range iterator.
         * @param range     The range to iterate over
         * @param position  The iterator's position in the range */
        iterator(const range<t>& range, const t& position)
            : range_(range)
            , cursor_(position)
        {
        }

        /** Move to the next element.
         ** The traversal order is x, y, z. */
        iterator& operator++()
        {
            assert(cursor_ != range_.last_);
            if (++cursor_[0] >= range_.last_[0]) {
                cursor_[0] = range_.first_[0];
                if (++cursor_[1] >= range_.last_[1]) {
                    cursor_[1] = range_.first_[1];
                    if (++cursor_[2] == range_.last_[2]) {
                        cursor_[1] = range_.last_[1];
                        cursor_[0] = range_.last_[0];
                    }
                }
            }
            return *this;
        }

        iterator operator++(int)
        {
            iterator tmp(*this);
            operator++();
            return tmp;
        }

        bool operator==(const iterator& compare) const
        {
            return cursor_ == compare.cursor_;
        }

        bool operator!=(const iterator& compare) const
        {
            return cursor_ != compare.cursor_;
        }

        t operator*() const { return cursor_; }
    };

public:
    friend class iterator;

    typedef t value_type;
    typedef iterator const_iterator;

public:
    /** Define a range from the origin to a given point.
     * @param r The end point for the range  */
    range(const t& r)
        : first_(0, 0, 0)
        , last_(r)
    {
    }

    /** Define a range between two values.
     * @param f     First value in the range
     * @param l     Last value in the range  */
    range(typename t::value_type f, typename t::value_type l)
        : first_(f, f, f)
        , last_(l, l, l)
    {
        assert(last_[0] >= first_[0]);
        assert(last_[1] >= first_[1]);
        assert(last_[2] >= first_[2]);
    }

    /** Define a range between two points.
     * @param f     First point in the range
     * @param l     Last point in the range */
    range(const t& f, const t& l)
        : first_(f)
        , last_(l)
    {
        assert(last_[0] >= first_[0]);
        assert(last_[1] >= first_[1]);
        assert(last_[2] >= first_[2]);
    }

    range(aabb<t> bbox)
        : first_(bbox.first)
        , last_(bbox.second)
    {
    }

    iterator begin() { return iterator(*this, first_); }
    iterator end() { return iterator(*this, last_); }

    const_iterator begin() const { return iterator(*this, first_); }
    const_iterator end() const { return iterator(*this, last_); }

    /** Get the height, width, and depth of the range. */
    t dimensions() const { return last_ - first_; }

    /** Get the number of voxels in this range. */
    size_t size() const { return prod(dimensions()); }

    /** Check if the range is empty. */
    bool empty() const
    {
        return first_[0] == last_[0] || first_[1] == last_[1]
               || first_[2] == last_[2];
    }

    /** Move the range by a given amount */
    range& operator+=(t shift)
    {
        first_ += shift;
        last_ += shift;
        return *this;
    }

    /** Move the range by a given amount */
    range& operator-=(t shift)
    {
        first_ -= shift;
        last_ -= shift;
        return *this;
    }

    range& operator/=(typename t::value_type div)
    {
        first_ /= div;
        last_ /= div;
        return *this;
    }

    range& operator*=(typename t::value_type mul)
    {
        first_ *= mul;
        last_ *= mul;
        return *this;
    }

    /** Expand the size of the range by 1 block */
    range& operator++()
    {
        last_ += t(1, 1, 1);
        return *this;
    }

    /** Contract the size of the range by 1 block */
    range& operator--()
    {
        last_ -= t(1, 1, 1);
        return *this;
    }

    const t& first() const { return first_; }
    const t& last() const { return last_; }

protected:
    t first_; /**< Start of the range, inclusive */
    t last_;  /**< End of the range, exclusive */
};

////////////////////////////////////////////////////////////////////////////

template <class type>
const range<type> operator+(range<type> lhs, range<type> rhs)
{
    return lhs += rhs;
}

template <class type>
const range<type> operator-(range<type> lhs, range<type> rhs)
{
    return lhs -= rhs;
}

template <class type>
const range<type> operator*(range<type> lhs, typename type::value_type rhs)
{
    return lhs *= rhs;
}

template <class type>
const range<type> operator/(range<type> lhs, typename type::value_type rhs)
{
    return lhs /= rhs;
}

////////////////////////////////////////////////////////////////////////////

/** Convenience function for making range objects.
 * @param from  Start of the range
 * @param to    End of the range (not inclusive) */
template <class type>
range<type> make_range(const type& from, const type& to)
{
    return range<type>(from, to);
}

/** Make a cube-shaped range around the origin.
 * @param size  The cube will be (2 * size + 1) in size.  A value of
 *              '0' will result in a range that only includes the origin.
 *              A value of '1' will return a range spanning a 3x3x3 cube. */
template <class type>
range<type> cube_range(typename type::value_type size)
{
    return range<type>(type(-size, -size, -size),
                       type(size + 1, size + 1, size + 1));
}

/** Make a range that is symmetrical around a given point.
 * @param origin  The center of the range
 * @param distance  The distance from the start and end points of the range to
 *                  the origin. */
template <class type>
range<type> surroundings(const type& origin,
                         const vector3<typename type::value_type>& distance)
{
    return range<type>(origin - distance,
                       origin + distance
                       + vector3<typename type::value_type>(1, 1, 1));
}

/** Make a range that is symmetrical around a given point.
 * @param origin  The center of the range
 * @param distance  The distance from the start and end points of the range to
 *                  the origin. */
template <class type>
range<type> surroundings(const type& origin,
                         typename type::value_type distance)
{
    return surroundings(origin, vector3<typename type::value_type>(
                                    distance, distance, distance));
}

/** Change a block range to a range in chunk coordinates. */
inline range<chunk_coordinates> to_chunk_range(range<world_coordinates> in)
{
    auto temp(--in / chunk_size);
    return ++temp;
}

/** Change a block range to a range in chunk coordinates. */
inline range<world_vector> to_chunk_range(range<world_vector> in)
{
    auto last(in.last());
    --last;
    range<world_vector> tmp(in.first() >> cnkshift, last >> cnkshift);
    return ++tmp;
}

/** Predefined range for iterating over every block in a \a chunk. */
static const range<chunk_index> every_block_in_chunk(0, chunk_size);

/** Predefined range for iterating over every block in a \a neighborhood. */
static const range<chunk_index>
every_block_in_neighborhood(-block_chunk_size, block_chunk_size * 2);

} // namespace hexa
