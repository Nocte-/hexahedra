//---------------------------------------------------------------------------
/// \file   rectangle.hpp
/// \brief  2-D rectangle class
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
#include <utility>
#include "vector2.hpp"

namespace hexa {

/** Simple 2-D rectangle class. */
template <typename t>
class rectangle : public std::pair<vector2<t>, vector2<t>>
{
public:
    typedef rectangle<t>                                self;
    typedef vector2<t>                                  value_type;
    typedef typename std::pair<value_type, value_type>  base;
    typedef t                                           dim_type;

    using base::first;
    using base::second;

public:
    rectangle () {}

    rectangle (value_type lower_right)
        : base (value_type(0, 0), lower_right)
    {}

    rectangle (value_type upper_left, value_type lower_right)
        : base (upper_left, lower_right)
    {}

    rectangle (dim_type L, dim_type T, dim_type R, dim_type B)
        : base (value_type(L, T), value_type(R, B))
    {}

    /** Check if the ordering of the two corners is correct.
     * @return  True if the upper left corner is indeed left of and above
     *          the lower right corner */
    bool is_correct () const
    {
        return first[0] <= second[0] && first[1] <= second[1];
    }

    /** Fix the rectangle, so the two vertices do point to the upper left
     ** and lower right corner respectively.  This function is only intended
     ** as a clean-up, and should be called after constructing a rectangle. */
    void fix ()
    {
        if (first[0] > second[0])
            std::swap (first[0], second[0]);

        if (first[1] > second[1])
            std::swap (first[1], second[1]);
    }

public:
	/**  The upper left corner of the rectangle.
	 * @return A vertex containing the upper left corner of the rectangle.*/
    const value_type top_left () const { return first; }

	/** The upper left corner of the rectangle.
	 * @return The upper left corner of the rectangle.*/
    value_type& top_left () { return first; }

    /** The lower right corner of the rectangle.
	 * @return A vertex containing the lower right corner of the rectangle.*/
    const value_type bottom_right () const { return second; }

    /** The lower right corner of the rectangle.
	 * @return The lower right corner of the rectangle.*/
    value_type& bottom_right () { return second; }

	/** The upper right corner of the rectangle.
	 * @return A vertex containing the upper right corner of the rectangle.*/
    const value_type top_right () const
    {
        value_type result (second);
        result[1] = first[1];
        return result;
    }

    /** The lower left corner of the rectangle.
	 * @return A vertex containing the lower left corner of the rectangle.*/
    const value_type bottom_left () const
    {
        value_type result (first);
        result[1] = second[1];
        return result;
    }

	/** The lowest x value of the rectangle.
	 * @return The lowest x value of the rectangle.*/
    const dim_type left () const { return first[0]; }

	/** The lowest x value of the rectangle.
	 * @return The lowest x value of the rectangle.*/
    dim_type& left () { return first[0]; }

    /** The highest x value of the rectangle.
	 * @return The highest x value of the rectangle.*/
	const dim_type right () const { return second[0]; }

    /** The highest x value of the rectangle.
	 * @return The highest x value of the rectangle.*/
	dim_type& right () { return second[0]; }

    /** The highest y value of the rectangle.
	 * @return The lowest y value of the rectangle.*/
	const dim_type top () const { return first[1]; }

    /** The highest y value of the rectangle.
	 * @return The lowest y value of the rectangle.*/
	dim_type& top ()  { return first[1]; }

    /** The lowest y value of the rectangle.
	 * @return The highest y value of the rectangle.*/
	const dim_type bottom () const { return second[1]; }

    /** The lowest y value of the rectangle.
	 * @return The highest y value of the rectangle.*/
	dim_type& bottom () { return second[1]; }

    /** Determine the width of the rectangle.
     * @return The width of the rectangle. */
    const dim_type width () const { return right() - left(); }

    /** Determine the height of the rectangle.
     * @return The height of the rectangle. */
    const dim_type height () const { return bottom() - top(); }

public:
    template <typename type>
    self& operator*= (type factor)
    {
        first *= factor;
        second *= factor;
        return *this;
    }

    template <typename type>
    self& operator/= (type factor)
    {
        first /= factor;
        second /= factor;
        return *this;
    }

    self& operator+= (const value_type& shift)
    {
        first += shift;
        second += shift;
        return *this;
    }

    self& operator-= (const value_type& shift)
    {
        first -= shift;
        second -= shift;
        return *this;
    }
};

//---------------------------------------------------------------------------

template <typename value_type, typename type>
const rectangle<value_type>
operator* (rectangle<value_type> rect, type factor)
{
    return rect *= factor;
}

template <typename value_type, typename type>
const rectangle<value_type>
operator* (type factor, rectangle<value_type> rect)
{
    return rect *= factor;
}

template <typename value_type, typename type>
const rectangle<value_type>
operator/ (rectangle<value_type> rect, type factor)
{
    return rect /= factor;
}

template <typename value_type>
const rectangle<value_type>
operator+ (rectangle<value_type> rect, value_type shift)
{
    return rect += shift;
}

template <class value_type>
const rectangle<value_type>
operator+ (rectangle<value_type> lhs, const rectangle<value_type>& rhs)
{
    return lhs += rhs;
}

template <class value_type>
const rectangle<value_type>
operator- (rectangle<value_type> lhs, const rectangle<value_type>& rhs)
{
    return lhs -= rhs;
}

} // namespace hexa

