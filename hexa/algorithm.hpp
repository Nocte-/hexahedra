//---------------------------------------------------------------------------
/// \file   algorithm.hpp
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
// Copyright 2012-2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>

namespace hexa
{

/** Integer divide, round negative down. */
template <typename T, typename Div>
const T divd(const T x, const Div d)
{
    return (x < 0) ? ((x - d + 1) / d) : (x / d);
}

/** Round a number to the nearest integer. */
template <typename T>
const int round(T x)
{
    return std::lround(x);
}

/** Integer specialization for round()
 * This may look stupid, but it is required for some algorithms that work
 * on both integer and floating point geometry. */
inline const int round(int in)
{
    return in;
}

/** Round a number towards zero. */
template <typename T>
const int round_to_zero(T x)
{
    return x >= 0 ? std::floor(x) : std::ceil(x);
}

/** Integer specialization for round_to_zero(). */
inline int round_to_zero(int in)
{
    return in;
}

/** Linear interpolation.
 * @param from  The start point
 * @param to    The end point
 * @param amount  Where to interpolate between the two.  A value of zero
 *                will return \a from, a value of one will return \a to,
 *                and everything in between will be interpolated. */
template <typename T>
const T lerp(const T& from, const T& to, double amount)
{
    return from * (1.0 - amount) + to * amount;
}

/** Return the value that is halfway between two given values.
 * @sa lerp() */
template <typename T>
const T halfway(const T& from, const T& to)
{
    return lerp(from, to, 0.5);
}

/** Return the difference between two values.
 * @return  The difference between a and b  (always positive) */
template <typename T>
auto diff(T a, T b) -> decltype(a - b)
{
    return (a > b) ? a - b : b - a;
}

/** Limit a given value to a minimum and maximum.
 * @pre min <= max
 * @return The value \a in, limited by \a min and \a max */
template <typename T>
const T clamp(T in, T min, T max)
{
    assert(min <= max);
    if (in < min)
        return min;
    if (in > max)
        return max;
    return in;
}

/** Clamp a value between zero and one. */
template <typename T>
const T saturate(T in)
{
    return clamp<T>(in, 0, 1);
}

template <typename T>
const T square(T in)
{
    return in * in;
}

/** Return -1, 0, or 1, depending on the sign of the input.
 * @return -1 if v < 0, 0 if v = 0, 1 if v > 0 */
template <typename T>
const int sign(T v)
{
    if (v > 0)
        return 1;

    if (v < 0)
        return -1;

    return 0;
}

/** smoothstep(t) = 3t^2 - 2t^3 */
template <typename T>
const T smoothstep(T v)
{
    return v * v * (T{3} - T{2} * v);
}

/** smootherstep(t) = 6t^5 - 15t^4 + 10t^3 */
template <typename T>
const T smootherstep(T v)
{
    return v * v * v * (v * (v * 6 - 15) + 10);
}

/** Check if two ranges overlap.
 * The edges are not included, so two ranges that only touch return false.
 * @pre start1 <= end1 && start2 <= end2 */
template <typename T>
bool ranges_overlap(T start1, T end1, T start2, T end2)
{
    assert(start1 <= end1);
    assert(start2 <= end2);
    return !(start2 >= end1 || start1 >= end2);
}

/** Check whether a value exists in a container. */
template <typename element, typename container>
bool exists_in(element elem, const container& c)
{
    return std::find(std::begin(c), std::end(c), elem) != c.end();
}

/** Erase all values from a container that satisfy a given predicate. */
template <typename container, typename Pred>
void erase_if(container& c, Pred op)
{
    for (auto i = std::begin(c); i != std::end(c);) {
        if (op(*i))
            i = c.erase(i);
        else
            ++i;
    }
}

/** Return the contents of a file as a string.
 * @throw std::runtime_error if the file could not be read */
inline const std::string file_contents(const boost::filesystem::path& file)
{
    std::string result;
    std::ifstream fs{file.string(), std::ios::binary};

    if (!fs)
        throw std::runtime_error((boost::format("cannot open %1% for reading")
                                  % file.string()).str());

    fs.seekg(0, std::ios::end);
    std::streamoff len = fs.tellg();
    fs.seekg(0);
    result.resize(static_cast<std::string::size_type>(len));
    fs.read(&*begin(result), len);
    return result;
}

/** Look up a value in a map, and throw an exception if it is not found.
 * @throw std::logic_error if \a key was not found in \a map
 *
 * Quick example:
 * @code

std::map<int, std::string> some_map;

some_map[3] = "three";

// This will print 'three'
std::cout << lookup(some_map, 3) << std::endl;
// This will throw a std::logic_error
std::cout << lookup(some_map, 8) << std::endl;

 * @endcode */
template <typename Map>
typename Map::mapped_type& lookup(Map& map, const typename Map::key_type& key)
{
    auto found = map.find(key);
    if (found == std::end(map))
        throw std::logic_error("hexa::lookup");

    return found->second;
}

/** Look up a value in a map, and return a default value if it is not found.
 * Quick example:
 * @code

std::map<int, std::string> some_map;

some_map[3] = "three";

// This will print 'three'
std::cout << lookup(some_map, 3, "unknown") << std::endl;
// This will print 'unknown'
std::cout << lookup(some_map, 8, "unknown") << std::endl;

 * @endcode */
template <typename Map>
typename Map::mapped_type&
lookup(Map& map, const typename Map::key_type& key,
       const typename Map::mapped_type& default_value)
{
    auto found = map.find(key);
    if (found == std::end(map))
        return default_value;

    return found->second;
}

template <typename T, typename Pred>
bool any_of(const T& p, Pred pred)
{
    return std::any_of(std::begin(p), std::end(p), pred);
}

//---------------------------------------------------------------------------

/** Calculate the product of all elements in a vector. */
template <typename T>
const typename T::value_type prod(const T& v)
{
    typename T::value_type result = 1;
    for (size_t i = 0; i < sizeof(T) / sizeof(typename T::value_type); ++i)
        result *= v[i];

    return result;
}

/** Calculate the dot product of two vectors. */
template <typename T>
const typename T::value_type dot_prod(const T& lhs, const T& rhs)
{
    typename T::value_type result = 0;
    for (size_t i = 0; i < sizeof(T) / sizeof(typename T::value_type); ++i)
        result += lhs[i] * rhs[i];

    return result;
}

/** Calculate the Manhattan length.
 *  This is the sum of the absolute values. */
template <typename T>
const typename T::value_type manhattan_length(const T& v)
{
    typename T::value_type result = 0;
    for (size_t i = 0; i < sizeof(T) / sizeof(typename T::value_type); ++i)
        result += std::abs(v[i]);

    return result;
}

/** Find the greatest length along any coordinate dimension. */
template <typename T>
const typename T::value_type chebyshev_length(const T& v)
{
    typename T::value_type result = 0;
    for (size_t i(0); i < sizeof(T) / sizeof(typename T::value_type); ++i)
        result = std::max(result, std::abs(v[i]));

    return result;
}

/** Calculate the squared length of a vector. */
template <typename T>
const double squared_length(const T& v)
{
    return dot_prod(v, v);
}

/** Calculate the length of a vector. */
template <typename T>
const double length(const T& v)
{
    return std::sqrt(squared_length(v));
}

/** Calculate the squared distance between two points. */
template <typename T>
const double squared_distance(const T& lhs, const T& rhs)
{
    return squared_length(lhs - rhs);
}

/** Calculate the distance between two points. */
template <typename T>
const double distance(const T& lhs, const T& rhs)
{
    return length(lhs - rhs);
}

/** Calculate the Manhattan distance between two points. */
template <typename T>
const typename T::value_type manhattan_distance(const T& lhs, const T& rhs)
{
    return manhattan_length(diff(lhs, rhs));
}

/** Calculate the greatest distance along any coordinate dimension. */
template <typename T>
const typename T::value_type chebyshev_distance(const T& lhs, const T& rhs)
{
    return chebyshev_length(diff(lhs, rhs));
}

/** Normalize a vector.
 *  The result is a vector with the same direction, and length 1.
 * \pre length(in) != 0
 * \post length(result) == 1 */
template <typename T>
const T normalize(const T& in)
{
    assert(length(in) != 0);
    return in / static_cast<typename T::value_type>(length(in));
}

/** Calculate the angle between two vectors. */
template <typename T>
const double angle(const T& a, const T& b)
{
    double length_prod = length(a) * length(b);
    assert(length_prod > 1e-8);
    return std::acos(clamp(dot_prod(a, b) / length_prod), -1.0, 1.0);
}

/** Project a vector onto another. */
template <typename T>
const T project_vector(const T& a, const T& b)
{
    assert(b != T::zero());
    return dot_prod(a, b) / squared_length(b) * b;
}

/** Minkowski sum of two sets.
 * The result is formed by adding each element in set a to each element in
 * set b. */
template <typename T>
const T minkowski_sum(const T& a, const T& b)
{
    T result;
    for (auto& i : a)
        for (auto& j : b)
            result.insert(i + j); // no emplace in gcc 4.7

    return result;
}

} // namespace hexa
