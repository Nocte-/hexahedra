//---------------------------------------------------------------------------
/// \file   hexa/algorithm.hpp
/// \brief  Collection of algorithms.
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
// Copyright 2012-2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>

namespace hexa {

/** Round a number to the nearest integer. */
template <typename type>
int round (type x) { return std::lround(x); }

/** Integer specialization for round()
 * This may look stupid, but it is required for some algorithms that work
 * on both integer and floating point geometry. */
inline int round (int in) { return in; }

/** Round a number towards zero. */
template <typename type>
int round_to_zero (type x)
{
    return x >= 0 ? std::floor(x) : std::ceil(x);
}

/** Integer specialization for round_to_zero(). */
inline int round_to_zero (int in) { return in; }

/** Linear interpolation.
 * \param from  The start point
 * \param to    The end point
 * \param amount  Where to interpolate between the two.  A value of zero
 *                will return \a from, a value of one will return \a to,
 *                and everything in between will be interpolated. */
template <typename type>
type lerp (const type& from, const type& to, double amount)
{
    return from * (1.0 - amount) + to * amount;
}

template <typename type>
type halfway (const type& from, const type& to)
{
    return lerp(from, to, 0.5);
}

/** Return the difference between two values.
 * \return  The difference between a and b  (always positive) */
template <typename type>
auto diff (type a, type b) -> decltype(a - b)
{
    return (a > b) ? a - b : b - a;
}

/** Limit a given value to a minimum and maximum.
 * \pre min <= max
 * \return The value \a in, limited by \a min and \a max */
template <typename type>
type clamp (type in, type min, type max)
{
    assert(min <= max);
    if (in < min) return min;
    if (in > max) return max;
    return in;
}

/** Clamp a value between zero and one. */
template <typename type>
type saturate (type in)
{
    return clamp<type>(in, 0, 1);
}

/** Return -1, 0, or 1, depending on the sign of the input.
 * \return -1 if v < 0, 0 if v = 0, 1 if v > 0 */
template <typename type>
int sign (type v)
{
    if (v > 0)
        return 1;

    if (v < 0)
        return -1;

    return 0;
}

/** smoothstep(t) = 3t^2 - 2t^3 */
template <typename type>
type smoothstep (type t)
{
    return t * t * (type(3) - type(2) * t);
}

/** smootherstep(t) = 6t^5 - 15t^4 + 10t^3 */
template <typename type>
type smootherstep (type t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

/** Check if two ranges overlap.
 * The edges are not included, so two ranges that only touch return false.
 * \pre start1 <= end1 && start2 <= end2 */
template <typename type>
bool ranges_overlap (type start1, type end1, type start2, type end2)
{
    assert(start1 <= end1);
    assert(start2 <= end2);
    return !(start2 >= end1 || start1 >= end2);
}

/** Check whether a value exists in a container. */
template<class element, class container>
bool exists_in (element elem, const container& c)
{
    return std::find(std::begin(c), std::end(c), elem) != c.end();
}

/** Erase all values from a container that satisfy a given predicate. */
template<class container, class pred>
void erase_if (container& c, pred op)
{
    for (auto i (std::begin(c)); i != std::end(c);)
    {
        if (op(*i))
            i = c.erase(i);
        else
            ++i;
    }
}

/** Return the contents of a file as a string.
 * \throw std::runtime_error if the file could not be read */
inline std::string
file_contents(const boost::filesystem::path& file)
{
    std::string result;
    std::ifstream fs (file.string(), std::ios::binary);

    if (!fs)
        throw std::runtime_error((boost::format("cannot open %1% for reading") % file.string()).str());

    fs.seekg(0, std::ios::end);
    std::streamoff len (fs.tellg());
    fs.seekg(0);
    result.resize(static_cast<std::string::size_type>(len));
    fs.read(&*begin(result), len);
    return result;
}


template <typename map_t>
typename map_t::mapped_type&
lookup (map_t& map, const typename map_t::key_type& key)
{
    auto found (map.find(key));
    if (found == std::end(map))
        throw std::logic_error("hexa::lookup");

    return found->second;
}

//---------------------------------------------------------------------------

/** Calculate the product of all elements in a vector. */
template <typename t>
typename t::value_type prod(const t& v)
{
    typename t::value_type result (1);
    for (size_t i (0); i < sizeof(t) / sizeof(typename t::value_type); ++i)
        result *= v[i];

    return result;
}

/** Calculate the dot product of two vectors. */
template <typename t>
typename t::value_type dot_prod(const t& lhs, const t& rhs)
{
    typename t::value_type result (0);
    for (size_t i (0); i < sizeof(t) / sizeof(typename t::value_type); ++i)
        result += lhs[i] * rhs[i];

    return result;
}

/** Calculate the Manhattan length. */
template <typename t>
typename t::value_type manhattan_length (const t& v)
{
    typename t::value_type result (0);
    for (size_t i (0); i < sizeof(t) / sizeof(typename t::value_type); ++i)
        result += std::abs(v[i]);

    return result;
}

/** Find the greatest length along any coordinate dimension. */
template <typename t>
typename t::value_type chebyshev_length (const t& v)
{
    typename t::value_type result (0);
    for (size_t i (0); i < sizeof(t) / sizeof(typename t::value_type); ++i)
        result = std::max(result, std::abs(v[i]));

    return result;
}

/** Calculate the squared length of a vector. */
template <typename t>
double squared_length (const t& v)
{
    return dot_prod(v, v);
}

/** Calculate the length of a vector. */
template <typename t>
double length (const t& v)
{
    return std::sqrt(squared_length(v));
}

/** Calculate the squared distance between two points. */
template <typename t>
double squared_distance (const t& lhs, const t& rhs)
{
    return squared_length(lhs - rhs);
}

/** Calculate the distance between two points. */
template <typename t>
double distance (const t& lhs, const t& rhs)
{
    return length(lhs - rhs);
}

/** Calculate the Manhattan distance between two points. */
template <typename t>
typename t::value_type
manhattan_distance (const t& lhs, const t& rhs)
{
    return manhattan_length(diff(lhs, rhs));
}

/** Calculate the greatest distance along any coordinate dimension. */
template <typename t>
typename t::value_type
chebyshev_distance (const t& lhs, const t& rhs)
{
    return chebyshev_length(diff(lhs, rhs));
}

/** Normalize a vector.
 * \pre length(in) != 0
 * \post length(result) == 1 */
template <typename t>
t normalize(const t& in)
{
    assert (length(in) != 0);
    return in / static_cast<typename t::value_type>(length(in));
}

/** Calculate the angle between two vectors. */
template <typename t>
double angle(const t& a, const t& b)
{
    double length_prod (length(a) * length(b));
    assert(length_prod > 1e-8);
    return std::acos(clamp(dot_prod(a, b) / length_prod), -1.0, 1.0);
}

/** Project a vector onto another. */
template <typename t>
t project_vector (const t& a, const t& b)
{
    assert(b != t::zero());
    return dot_prod(a, b) / squared_length(b) * b;
}

/** Minkowski sum of two sets. */
template <typename t>
t minkowski_sum (const t& a, const t& b)
{
    t result;
    for (auto& i : a)
        for (auto& j : b)
            result.insert(i + j); // no emplace in gcc 4.7

    return result;
}

} // namespace hexa

