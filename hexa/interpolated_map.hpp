//---------------------------------------------------------------------------
/// \file   hexa/interpolated_map.hpp
/// \brief  Adds linear interpolated lookups to a sorted associative container
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

#include <cassert>
#include <iterator>
#include <map>

#include <initializer_list>

namespace hexa
{

/** An adapter that adds linear interpolation to a sorted associative
 ** container.
 *
 *  For a map that has the keys {a0, a1, ..., an }, the valid lookups are in
 *  the range  [a0, an).
 *
 * Example:
 * @code
#include "color.hpp"
#include "interpolated_map.hpp"

using namespace hexa;

interpolated_map<float, color> gradient;

gradient[0] = color(1, 0, 0);
gradient[4] = color(0, 1, 0);
gradient[5] = color(0, 0, 1);

// gradient(0) returns { 1, 0, 0, }
// gradient(2) returns { 0.5, 0.5, 0 }
// gradient(4.5) returns { 0, 0.5, 0.5 }

// gradient.lookup(1).interpolate is 0.25, the normalized position between 0
and 4

 * @endcode
 */
template <class Key = float, class T = float, class Impl = std::map<Key, T>>
class interpolated_map
{
private:
    typedef Impl map_t;

public:
    typedef Key key_type;
    typedef T mapped_type;

    typedef typename map_t::value_type value_type;
    typedef typename map_t::iterator iterator;
    typedef typename map_t::const_iterator const_iterator;
    typedef typename map_t::size_type size_type;
    typedef typename map_t::allocator_type allocator_type;

public:
    interpolated_map() {}
    interpolated_map(std::initializer_list<value_type> i)
        : map_(i)
    {
    }

    interpolated_map(interpolated_map&& m)
        : map_(std::move(m.map_))
    {
    }

    interpolated_map& operator=(interpolated_map&& m)
    {
        if (this != &m)
            map_ = std::move(m.map_);

        return *this;
    }

public:
    /** The result of a lookup is the two boundaries, and the relative
     ** interpolation between them. */
    template <typename Iter>
    struct result
    {
        Iter lower; /**< Lower boundary. */
        Iter upper; /**< Upper boundary. */
        key_type
        interpolated; /**< Interpolation between the boundaries, range [0,1) */

        result(Iter l, Iter u, key_type i)
            : lower{l}
            , upper{u}
            , interpolated{i}
        {
        }

        operator mapped_type() const
        {
            return mapped_type((1 - interpolated) * lower->second
                               + interpolated * upper->second);
        }
    };

public:
    result<iterator> lookup(key_type value)
    {
        assert(size() >= 2);

        iterator upper(map_.lower_bound(value));
        assert(upper != begin());
        assert(upper != end());
        iterator lower(upper);
        --lower;

        return result<iterator>(lower, upper, (value - lower->first)
                                              / (upper->first - lower->first));
    }

    result<const_iterator> lookup(key_type value) const
    {
        assert(size() >= 2);

        const_iterator upper(map_.lower_bound(value));
        assert(upper != begin());
        assert(upper != end());
        const_iterator lower(upper);
        --lower;

        return result<const_iterator>(lower, upper,
                                      (value - lower->first)
                                      / (upper->first - lower->first));
    }

    mapped_type operator()(key_type value) const { return lookup(value); }

public:
    bool empty() const { return map_.empty(); }
    size_type size() const { return map_.size(); }
    iterator begin() { return map_.begin(); }
    const_iterator begin() const { return map_.begin(); }
    iterator end() { return map_.end(); }
    const_iterator end() const { return map_.end(); }

    mapped_type& operator[](key_type index) { return map_[index]; }

    iterator erase(iterator pos) { return map_.erase(pos); }

    iterator erase(iterator first, iterator last)
    {
        return map_.erase(first, last);
    }

    key_type range() const
    {
        assert(!empty());
        return std::prev(end())->first;
    }

private:
    map_t map_;
};

} // namespace hexa
