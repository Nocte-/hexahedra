//---------------------------------------------------------------------------
/// \file   hexa/client/gui/constraints.hpp
/// \brief  Convenience functions for creating constraints
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
// Copyright 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <rhea/constraint.hpp>
#include "widget.hpp"

namespace hexa
{
namespace gui
{

/** Combine a number of constraints lists to one single list. */
inline rhea::constraint_list
combine(std::initializer_list<rhea::constraint_list> l)
{
    rhea::constraint_list result;

    for (auto& elem : l)
        result.splice(result.end(), elem);

    return result;
}

inline rhea::constraint_list place_left_of(const widget& a, const widget& b,
                                           double space = 0.0)
{
    return {a.right() + space <= b.left()};
}

inline rhea::constraint_list place_right_of(const widget& a, const widget& b,
                                            double space = 0.0)
{
    return {a.left() - space >= b.right()};
}

inline rhea::constraint_list place_above(const widget& a, const widget& b,
                                         double space = 0.0)
{
    return {a.bottom() + space <= b.top()};
}

inline rhea::constraint_list place_below(const widget& a, const widget& b,
                                         double space = 0.0)
{
    return {a.top() - space >= b.bottom()};
}

inline rhea::constraint_list center_h(const widget& a, const widget& b)
{
    return {a.center_h() == b.center_h()};
}

inline rhea::constraint_list center_v(const widget& a, const widget& b)
{
    return {a.center_v() == b.center_v()};
}

inline rhea::constraint_list center(const widget& a, const widget& b)
{
    return combine({center_h(a, b), center_v(a, b)});
}

inline rhea::constraint_list equal_height(const widget& a, const widget& b)
{
    return {a.height() == b.height()};
}

inline rhea::constraint_list equal_width(const widget& a, const widget& b)
{
    return {a.width() == b.width()};
}

inline rhea::constraint_list equal_size(const widget& a, const widget& b)
{
    return combine({equal_width(a, b), equal_height(a, b)});
}

inline rhea::constraint_list align_left(const widget& a, const widget& b)
{
    return {a.left() == b.left()};
}

inline rhea::constraint_list align_top(const widget& a, const widget& b)
{
    return {a.top() == b.top()};
}

inline rhea::constraint_list align_right(const widget& a, const widget& b)
{
    return {a.right() == b.right()};
}

inline rhea::constraint_list align_bottom(const widget& a, const widget& b)
{
    return {a.bottom() == b.bottom()};
}

inline rhea::constraint_list keep_inside(const widget& a, const widget& b)
{
    return {a.left() >= b.left(), a.top() >= b.top(), a.right() <= b.right(),
            a.bottom() <= b.bottom()};
}

inline rhea::constraint_list frame(const widget& a, const widget& b, double h,
                                   double v)
{
    return {a.left() + h == b.left(), a.top() + v == b.top(),
            a.right() - h == b.right(), a.bottom() - v == b.bottom()};
}

//--------------------------------------------------------------------------

/** Create a list of constraints by applying a constraint to multiple widgets.
 *  For a constraint f and a list of widgets (a, b, c, d, ...), this will
 *  result in a list of constraints (f(a, b), f(b, c), f(c, d), ...)
 *  An empty list or a list with only one widget will give an empty result. */
inline rhea::constraint_list template <typename op>
chain(std::initializer_list<const widget&> wl, op func)
{
    rhea::constraint_list result;

    if (wl.size() < 2)
        return result;

    auto i(std::begin(wl));
    for (auto j(std::next(i)); j != std::end(wl); ++i, ++j)
        result.splice(result.end(), func(*i, *j));

    return result;
}

/** Chain a list of widgets with multiple functions. */
inline rhea::constraint_list template <typename op, typename... op_tail>
chain(std::initializer_list<const widget&> wl, op func, op_tail... tail)
{
    auto result(chain(wl, func));
    result.insert(result.end(), chain(wl, tail));
    return result;
}

//--------------------------------------------------------------------------

/** Pack a list of widgets horizontally. */
inline rhea::constraint_list pack_h(std::initializer_list<const widget&> wl,
                                    double spacing = 0.0)
{
    auto partial_app([=](const widget& a, const widget& b) {
        return place_left_of(a, b, spacing);
    });

    return chain(wl, partial_app, align_top, equal_height);
}

/** Pack a list of widgets vertically. */
inline rhea::constraint_list pack_v(std::initializer_list<const widget&> wl,
                                    double spacing = 0.0)
{
    auto partial_app([=](const widget& a, const widget& b) {
        return place_below(a, b, spacing);
    });

    return chain(wl, partial_app, align_left, equal_width);
}

/** Pack a list of widgets horizontally, and make them all the same width. */
inline rhea::constraint_list
pack_equal_h(std::initializer_list<const widget&> wl, double spacing = 0.0)
{
    return combine({pack_h(wl, spacing), chain(wl, equal_width)});
}

/** Pack a list of widgets vertically, and make them all the same height. */
inline rhea::constraint_list
pack_equal_v(std::initializer_list<const widget&> wl, double spacing = 0.0)
{
    return combine({pack_v(wl, spacing), chain(wl, equal_height)});
}
}
} // namespace hexa::gui
