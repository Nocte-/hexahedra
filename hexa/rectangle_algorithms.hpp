//---------------------------------------------------------------------------
/// \file   rectangle_algorithms.hpp
/// \brief  Functions for manipulating rectangles
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

#include <algorithm>
#include <cassert>
#include <iterator>
#include <vector>
#include "algorithm.hpp"
#include "rectangle.hpp"

namespace hexa
{

/** Calculate the area of a rectangle.
 * @param rect  The rectangle
 * @return  The product of the rectangle's width and height */
template <typename t>
t area(const rectangle<t>& rect)
{
    return rect.width() * rect.height();
}

/** Calculate the circumfence of a rectangle.
 * @param rect  The rectangle
 * @return  The sum of the length of the rectangle's sides */
template <typename t>
t circumference(const rectangle<t>& rect)
{
    return rect.width() * 2 + rect.height() * 2;
}

/** Calculate the middle of a rectangle.
 * @param rect  The rectangle
 * @return  The point halfway two opposite corners of the rectangle */
template <typename t>
typename rectangle<t>::value_type center(const rectangle<t>& rect)
{
    return halfway(rect.first, rect.second);
}

/** Determine if a point is located inside a rectangle.
 *  As a precondition, rect.is_correct() must be true.  If this is not
 *  the case, call rect.fix() first.
 * @param point The point to test for
 * @param rect  The rectangle
 * @return  True if the point is located inside the rectangle */
template <typename t>
bool point_in_rectangle(const typename rectangle<t>::value_type& point,
                        const rectangle<t>& rect)
{
    assert(rect.is_correct());

    return point[0] > rect.left() && point[1] > rect.top()
           && point[0] < rect.right() && point[1] < rect.bottom();
}

/** Determine if a point is located on a rectangle.
 *  As a precondition, rect.is_correct() must be true.  If this is not
 *  the case, call rect.fix() first.
 * @param point The point to test for
 * @param rect  The rectangle
 * @return  True if the point is located on the rectangle */
template <typename t>
bool point_on_rectangle(const typename rectangle<t>::value_type& point,
                        const rectangle<t>& rect)
{
    assert(rect.is_correct());
    return point[0] >= rect.left() && point[1] >= rect.top()
           && point[0] <= rect.right() && point[1] <= rect.bottom();
}

/** Determine if two rectangles overlap.
 *  As a precondition, is_correct() must be true for both rectangles.  If
 *  this is not the case, call fix() first.
 * @param lhs  The first rectangle
 * @param rhs  The second rectangle
 * @return  True if lhs and rhs overlap */
template <typename t>
bool rectangles_are_overlapping(const rectangle<t>& lhs,
                                const rectangle<t>& rhs)
{
    assert(lhs.is_correct());
    assert(rhs.is_correct());
    return !(rhs.right() <= lhs.left() || rhs.left() >= lhs.right()
             || rhs.bottom() <= lhs.top() || rhs.top() >= lhs.bottom());
}

/** Determine if two rectangles touch.
 *  As a precondition, is_correct() must be true for both rectangles.  If
 *  this is not the case, call fix() first.
 * @param lhs The first rectangle
 * @param rhs The second rectangle
 * @return  True if lhs and rhs touch somewhere */
template <typename t>
bool rectangles_are_touching(const rectangle<t>& lhs, const rectangle<t>& rhs)
{
    assert(lhs.is_correct());
    assert(rhs.is_correct());
    return !(rhs.right() < lhs.left() || rhs.left() > lhs.right()
             || rhs.bottom() < lhs.top() || rhs.top() > lhs.bottom());
}

/** Determine if one rectangle is completely inside another.
 * @param outer The outer rectangle
 * @param inner The inner rectangle
 * @return True if inner is completely inside outer */
template <typename t>
bool is_inside(const rectangle<t>& inner, const rectangle<t>& outer)
{
    assert(inner.is_correct());
    assert(outer.is_correct());
    return inner.left() >= outer.left() && inner.top() >= outer.top()
           && inner.right() <= outer.right()
           && inner.bottom() <= outer.bottom();
}

/** Determine the intersection rectangle from two other rectangles.
 *  As a precondition, is_correct() must be true for both rectangles.  If
 *  this is not the case, call fix() first.
 * @param lhs  The first rectangle
 * @param rhs  The second rectangle
 * @return  The intersection bewtween lhs and rhs.  If the returned rectangle
 *          is not correct (!is_correct()), no intersection was found. */
template <typename t>
rectangle<t> make_intersection(const rectangle<t>& lhs,
                               const rectangle<t>& rhs)
{
    assert(lhs.is_correct());
    assert(rhs.is_correct());

    return rectangle<t>(std::max(lhs.left(), rhs.left()),
                        std::max(lhs.top(), rhs.top()),
                        std::min(lhs.right(), rhs.right()),
                        std::min(lhs.bottom(), rhs.bottom()));
}

/** Determine the bounding rectangle from two other rectangles.
 *  As a precondition, is_correct() must be true for both rectangles.  If
 *  this is not the case, call fix() first.
 * @param lhs  The first rectangle
 * @param rhs  The second rectangle
 * @return  A rectangle that contains both lhs and rhs completely. */
template <typename t>
rectangle<t> make_union(const rectangle<t>& lhs, const rectangle<t>& rhs)
{
    assert(lhs.is_correct());
    assert(rhs.is_correct());

    return rectangle<t>(std::min(lhs.left(), rhs.left()),
                        std::min(lhs.top(), rhs.top()),
                        std::max(lhs.right(), rhs.right()),
                        std::max(lhs.bottom(), rhs.bottom()));
}

/** Determine the bounding rectangle of a set of points.
 * @pre first != last
 * @param first     Start of the set of points
 * @param last      End of the set of points
 * @return A rectangle for which point_on_rectangle() is true for every
 *         point in the input set. */
template <class iterator>
rectangle<typename std::iterator_traits<iterator>::value_type::value_type>
bounding_rectangle(iterator first, iterator last)
{
    typedef typename std::iterator_traits<iterator>::value_type::value_type t;
    assert(first != last);
    rectangle<t> result(*first, *first);
    for (++first; first != last; ++first) {
        if (result.left() > (*first)[0])
            result.left() = (*first)[0];
        else if (result.right() < (*first)[0])
            result.right() = (*first)[0];

        if (result.top() > (*first)[1])
            result.top() = (*first)[1];
        else if (result.bottom() < (*first)[1])
            result.bottom() = (*first)[1];
    }

    return result;
}

/** Determine the bounding rectangle of a set of points.
 * @pre !points.empty()
 * @param points    The input set of points
 * @return A rectangle for which point_on_rectangle() is true for every
 *         point in the input set. */
template <class container>
rectangle<typename container::value_type::value_type>
bounding_rectangle(const container& points)
{
    return bounding_rectangle(points.begin(), points.end());
}

/** Inflate a rectangle by a given amount.
 *  As a precondition, rect.is_correct() must be true.  If this is not
 *  the case, call rect.fix() first.
 * @param in      The rectangle
 * @param amount  The vector that will be used to move the corners */
template <typename t>
rectangle<t> inflate(rectangle<t> in, typename rectangle<t>::value_type amount)
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
template <typename t>
rectangle<t> inflate(rectangle<t> in, t amount)
{
    assert(in.is_correct());
    typename rectangle<t>::value_type shift(amount, amount);
    in.first -= shift;
    in.second += shift;
    return in;
}

/** Turn a rectangle into a polygon.
 * @param rect  The source rectangle
 * @param out   The destination
 * @return  A copy of the used output iterator */
template <typename t, class output_iterator>
output_iterator make_polygon(const rectangle<t>& rect, output_iterator out)
{
    *out++ = rect.top_left();
    *out++ = rect.top_right();
    *out++ = rect.bottom_right();
    *out++ = rect.bottom_left();

    return out;
}

/** Turn a rectangle into a polygon.
 * @param rect  The source rectangle
 * @return  The same rectangle, encoded as a polygon. */
template <typename t>
std::vector<t> make_polygon(const rectangle<t>& rect)
{
    std::vector<t> temp(4);
    make_polygon(rect, temp.begin());
    return temp;
}

/** Subdivide a rectangle into four rectangles.
 * @param src   The source rectangle
 * @param at    The point of subdivision
 * @param out   The destination
 * @pre point_in_rectangle (at, src) */
template <typename t, class output_iterator>
output_iterator subdivide(const rectangle<t>& src,
                          const typename rectangle<t>::value_type& at,
                          output_iterator out)
{
    *out++ = rectangle<t>(src.first, at);
    *out++ = rectangle<t>(at[0], src.top(), src.right(), at[1]);
    *out++ = rectangle<t>(src.left(), at[1], at[0], src.bottom());
    *out++ = rectangle<t>(at, src.second);
    return out;
}

/** Resize an existing rectangle to include a given point.
 * @param in    Source rectangle
 * @param p     Target rectangle
 * @return A rectangle r that satisfies rectangles_are_overlapping (src, r)
 *         and point_on_rectangle (p, r) */
template <typename t>
rectangle<t> include(rectangle<t> in,
                     const typename rectangle<t>::value_type& p)
{
    if (p[0] < in.left())
        in.left() = p[0];
    else if (p[0] > in.right())
        in.right() = p[0];

    if (p[1] < in.top())
        in.top() = p[1];
    else if (p[1] > in.bottom())
        in.bottom() = p[1];

    return in;
}

} // namespace hexa
