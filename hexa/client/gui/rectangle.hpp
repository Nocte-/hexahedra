//---------------------------------------------------------------------------
/// \file   client/gui/rectangle.hpp
/// \brief  A simple rectangle
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
// Copyright 2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include <limits>

namespace gui
{

template <typename Point>
class rectangle
{
public:
    typedef rectangle<Point> self;
    typedef Point value_type;
    typedef typename Point::value_type dim_type;

    Point first;
    Point second;

public:
    rectangle() {}

    rectangle(value_type upper_left, value_type lower_right)
        : first{upper_left}
        , second{lower_right}
    {
    }

    rectangle(dim_type L, dim_type T, dim_type R, dim_type B)
        : first{L, T}
        , second{R, B}
    {
    }

    /** Create an invalid rectangle.
     *  This can be useful to flag an uninitialized state, or as a starting
     *  rectangle for determining bounding boxes. */
    static rectangle invalid()
    {
        auto l = std::numeric_limits<dim_type>::lowest();
        auto h = std::numeric_limits<dim_type>::max();
        return rectangle{h, h, l, l};
    }

    /** Check if the ordering of the two corners is correct.
     * @return  True if the upper left corner is indeed left of and above
     *          the lower right corner */
    bool is_correct() const
    {
        return first[0] <= second[0] && first[1] <= second[1];
    }

    /** Fix the rectangle, so the two vertices do point to the upper left
     ** and lower right corner respectively.  This function is only intended
     ** as a clean-up, and should be called after constructing a rectangle. */
    void fix()
    {
        if (first[0] > second[0])
            std::swap(first[0], second[0]);

        if (first[1] > second[1])
            std::swap(first[1], second[1]);
    }

public:
    const value_type top_left() const { return first; }
    value_type& top_left() { return first; }

    const value_type bottom_right() const { return second; }
    value_type& bottom_right() { return second; }

    const value_type top_right() const
    {
        value_type result{second};
        result[1] = first[1];
        return result;
    }

    const value_type bottom_left() const
    {
        value_type result{first};
        result[1] = second[1];
        return result;
    }

    const dim_type left() const { return first[0]; }
    dim_type& left() { return first[0]; }

    const dim_type right() const { return second[0]; }
    dim_type& right() { return second[0]; }

    const dim_type top() const { return first[1]; }
    dim_type& top() { return first[1]; }

    const dim_type bottom() const { return second[1]; }
    dim_type& bottom() { return second[1]; }

    const dim_type width() const { return right() - left(); }
    const dim_type height() const { return bottom() - top(); }

public:
    bool operator==(const rectangle<value_type>& cmp) const
    {
        return first == cmp.first && second == cmp.second;
    }

    bool operator!=(const rectangle<value_type>& cmp) const
    {
        return !operator==(cmp);
    }

    template <typename type>
    self& operator*=(type factor)
    {
        first *= factor;
        second *= factor;
        return *this;
    }

    template <typename type>
    self& operator/=(type factor)
    {
        first /= factor;
        second /= factor;
        return *this;
    }

    self& operator+=(const value_type& shift)
    {
        first += shift;
        second += shift;
        return *this;
    }

    self& operator-=(const value_type& shift)
    {
        first -= shift;
        second -= shift;
        return *this;
    }
};

//---------------------------------------------------------------------------

/** Calculate the area of a rectangle.
 * @param rect  The rectangle
 * @return  The product of the rectangle's width and height */
template <typename T>
auto area(const rectangle<T>& r) -> decltype(r.width() * r.height())
{
    return r.width() * r.height();
}

/** Calculate the middle of a rectangle.
 * @param rect  The rectangle
 * @return  The point halfway two opposite corners of the rectangle */
template <typename T>
T center(const rectangle<T>& rect)
{
    return (rect.first + rect.second) * 0.5;
}

/** Determine if a point is located inside a rectangle.
 * @pre rect.is_correct()
 * @param point The point to test for
 * @param rect  The rectangle
 * @return  True if the point is located inside the rectangle */
template <typename T>
bool point_in_rectangle(const T& point, const rectangle<T>& rect)
{
    assert(rect.is_correct());
    return point[0] > rect.left() && point[1] > rect.top()
           && point[0] < rect.right() && point[1] < rect.bottom();
}

/** Determine if a point is located on a rectangle.
 * @pre rect.is_correct()
 * @param point The point to test for
 * @param rect  The rectangle
 * @return  True if the point is located on the rectangle */
template <typename T>
bool point_on_rectangle(const T& point, const rectangle<T>& rect)
{
    assert(rect.is_correct());
    return point[0] >= rect.left() && point[1] >= rect.top()
           && point[0] <= rect.right() && point[1] <= rect.bottom();
}

/** Determine if two rectangles overlap.
 * @pre lhs.is_correct() && rhs.is_correct()
 * @param lhs  The first rectangle
 * @param rhs  The second rectangle
 * @return  True if lhs and rhs overlap */
template <typename T>
bool are_overlapping(const rectangle<T>& lhs, const rectangle<T>& rhs)
{
    assert(lhs.is_correct());
    assert(rhs.is_correct());
    return !(rhs.right() <= lhs.left() || rhs.left() >= lhs.right()
             || rhs.bottom() <= lhs.top() || rhs.top() >= lhs.bottom());
}

/** Determine if two rectangles touch.
 * @pre lhs.is_correct() && rhs.is_correct()
 * @param lhs The first rectangle
 * @param rhs The second rectangle
 * @return  True if lhs and rhs touch somewhere */
template <typename T>
bool are_touching(const rectangle<T>& lhs, const rectangle<T>& rhs)
{
    assert(lhs.is_correct());
    assert(rhs.is_correct());
    return !(rhs.right() < lhs.left() || rhs.left() > lhs.right()
             || rhs.bottom() < lhs.top() || rhs.top() > lhs.bottom());
}

/** Determine if one rectangle is completely inside another.
 * @pre lhs.is_correct() && rhs.is_correct()
 * @param outer The outer rectangle
 * @param inner The inner rectangle
 * @return True if inner is completely inside outer */
template <typename T>
bool is_inside(const rectangle<T>& inner, const rectangle<T>& outer)
{
    assert(inner.is_correct());
    assert(outer.is_correct());
    return inner.left() >= outer.left() && inner.top() >= outer.top()
           && inner.right() <= outer.right()
           && inner.bottom() <= outer.bottom();
}

/** Determine the intersection rectangle of two rectangles.
 * @pre lhs.is_correct() && rhs.is_correct()
 * @param lhs  The first rectangle
 * @param rhs  The second rectangle
 * @return  The intersection bewtween lhs and rhs.  If the returned rectangle
 *          is not correct (!is_correct()), no intersection was found. */
template <typename T>
rectangle<T> make_intersection(const rectangle<T>& lhs,
                               const rectangle<T>& rhs)
{
    assert(lhs.is_correct());
    assert(rhs.is_correct());
    return {std::max(lhs.left(), rhs.left()), std::max(lhs.top(), rhs.top()),
            std::min(lhs.right(), rhs.right()),
            std::min(lhs.bottom(), rhs.bottom())};
}

/** Determine the bounding rectangle of two rectangles.
 * @pre lhs.is_correct() && rhs.is_correct()
 * @param lhs  The first rectangle
 * @param rhs  The second rectangle
 * @return  A rectangle that contains both lhs and rhs completely. */
template <typename T>
rectangle<T> make_union(const rectangle<T>& lhs, const rectangle<T>& rhs)
{
    assert(lhs.is_correct());
    assert(rhs.is_correct());
    return {std::min(lhs.left(), rhs.left()), std::min(lhs.top(), rhs.top()),
            std::max(lhs.right(), rhs.right()),
            std::max(lhs.bottom(), rhs.bottom())};
}

/** Inflate a rectangle by a given amount.
 * @param in      The rectangle
 * @param amount  The vector that will be used to move the corners */
template <typename T>
rectangle<T> inflate(rectangle<T> in, T amount)
{
    assert(in.is_correct());
    in.first -= amount;
    in.second += amount;
    return in;
}

/** Inflate a rectangle by a given amount.
 *  As a precondition, rect.is_correct() must be true.  If this is not
 *  the case, call rect.fix() first.
 * @param in      The rectangle
 * @param amount  The distance that will be used to move the corners */
template <typename T>
rectangle<T> inflate(rectangle<T> in, typename rectangle<T>::dim_type amount)
{
    assert(in.is_correct());
    T shift{amount, amount};
    in.first -= shift;
    in.second += shift;
    return in;
}

//---------------------------------------------------------------------------

template <typename value_type, typename type>
const rectangle<value_type> operator*(rectangle<value_type> rect, type factor)
{
    return rect *= factor;
}

template <typename value_type, typename type>
const rectangle<value_type> operator*(type factor, rectangle<value_type> rect)
{
    return rect *= factor;
}

template <typename value_type, typename type>
const rectangle<value_type> operator/(rectangle<value_type> rect, type factor)
{
    return rect /= factor;
}

template <typename value_type>
const rectangle<value_type> operator+(rectangle<value_type> rect,
                                      value_type shift)
{
    return rect += shift;
}

template <class value_type>
const rectangle<value_type> operator+(rectangle<value_type> lhs,
                                      const rectangle<value_type>& rhs)
{
    return lhs += rhs;
}

template <class value_type>
const rectangle<value_type> operator-(rectangle<value_type> lhs,
                                      const rectangle<value_type>& rhs)
{
    return lhs -= rhs;
}

} // namespace gui
