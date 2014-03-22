//---------------------------------------------------------------------------
/// \file   hexa/distance_sorted_map.hpp
/// \brief  Manage a distance-sorted map
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

#include <cmath>
#include <unordered_map>
#include <vector>

#include <boost/signals2.hpp>

#include <hexa/basic_types.hpp>
#include <hexa/trace.hpp>

namespace hexa {

template <typename t>
class distance_sorted_map
{
    /// Collection of objects with the same distance.
    typedef std::unordered_map<chunk_coordinates, t> shell_t;

    /// Shells are kept sorted by their Manhattan distance.
    typedef std::vector<shell_t> sorted_t;

public:
    //@{
    /// Public typedefs.
    typedef chunk_coordinates               key_type;
    typedef typename shell_t::value_type    value_type;
    typedef t                               mapped_type;
    //@}

public:
    /** Signals an item has just been added. */
    boost::signals2::signal<void(key_type, mapped_type&)> on_new;

    /** Signals an item is about to be updated. */
    boost::signals2::signal<void(key_type, mapped_type&)> on_before_update;

    /** Signals an item has just been updated. */
    boost::signals2::signal<void(key_type, mapped_type&)> on_after_update;

    /** Signals an item is about to be removed. */
    boost::signals2::signal<void(key_type, mapped_type&)> on_remove;

public:
    distance_sorted_map()
        : view_dist_(0)
    { }

    size_t  size() const
    {
        size_t sum (0);
        for (auto& shell : data_)
            sum += shell.size();
        return sum;
    }

    bool    empty() const
    {
        return data_.empty();
    }

public:

    /** Check if a given chunk is inside the view radius. */
    bool
    is_inside (chunk_coordinates pos) const
    {
        return squared_distance(pos, pos_) <= sq_view_dist_;
    }

    /** Query the view radius, measured in chunks. */
    size_t
    view_radius () const
    {
        return view_dist_;
    }

    /** Change the view radius, measured in chunks. */
    void
    view_radius (size_t new_radius)
    {
        bool erase (false);

        if (new_radius < view_dist_)
        {
            erase = true;
            for_each([&](value_type& p)
            {
                if (!is_inside(p.first))
                    on_remove(p.first, p.second);
            });
        }

        view_dist_ = new_radius;
        sq_view_dist_ = new_radius * new_radius;

        // Convert from inscribed sphere radius to octahedron size.
        // (Since we sort by Manhattan distance, the octahedron
        // is our "sphere".)
        size_t octahedron_radius (static_cast<size_t>(std::ceil(new_radius * 1.7321)));
        data_.resize(octahedron_radius);

        if (erase)
        {
            for (auto& shell : data_)
            {
                for (auto i (shell.begin()); i != shell.end(); ++i)
                {
                    if (!is_inside(i->first))
                        shell.erase(i);
                }
            }
        }
    }

    /** Move the center to a new position.
     *  All items are sorted by distance to this new center.  If any element
     *  is now too far away, it is deleted automatically. */
    void
    center (chunk_coordinates new_pos)
    {
        if (pos_ == new_pos)
            return;

        pos_ = new_pos;

        sorted_t temp (data_.size());
        for (auto& l : data_)
        {
            for (auto& p : l)
            {
                if (is_inside(p.first))
                    temp[index(p.first)][p.first] = std::move(p.second);
                else
                    on_remove(p.first, p.second);
            }
        }
        data_.swap(temp);
    }

    chunk_coordinates
    center() const
    {
        return pos_;
    }

    /** Add or update an item.
     * @param pos   The item's position
     * @param item  The item itself
     * @return true iff the item was set */
    bool
    set (chunk_coordinates pos, mapped_type&& item)
    {
        if (!is_inside(pos))
            return false;

        auto& shell (data_[index(pos)]);
        auto found (shell.find(pos));
        if (found == shell.end())
        {
            auto result (shell.emplace(pos, std::move(item)));
            on_new(pos, result.first->second);
            return true;
        }
        else
        {
            on_before_update(pos, found->second);
            found->second = std::move(item);
            on_after_update(pos, found->second);
            return true;
        }
    }

    bool
    has (chunk_coordinates pos) const
    {
        return is_inside(pos) && data_[index(pos)].count(pos) != 0;
    }

    const mapped_type&
    get (chunk_coordinates pos) const
    {
        const shell_t& sh (data_[index(pos)]);
        auto found (sh.find(pos));
        if (found == sh.end())
            throw std::runtime_error("invalid index");

        return found->second;
    }

    /** Remove an item.
     * @param pos   The item's position
     * @return true iff the item was removed */
    bool
    remove (chunk_coordinates pos)
    {
        size_t dist (index(pos));
        if (dist >= data_.size() || !is_inside(pos))
            return false;

        auto& shell (data_[dist]);
        auto found (shell.find(pos));
        if (found == shell.end())
            return false;

        on_remove(pos, found->second);
        shell.erase(pos);
        return true;
    }

    /** Apply a function to every element, in a near-to-far order.
     *  The function is passed a pair_t. */
    template <typename func>
    void
    for_each (func op) const
    {
        for (auto& shell : data_)
        {
            for (auto& elem : shell)
                op(elem);
        }
    }

    template <typename func>
    void
    for_each (func op)
    {
        for (auto& shell : data_)
        {
            for (auto& elem : shell)
                op(elem);
        }
    }

    /** Apply a function to every element, in a far-to-near order.
     *  The function is passed a pair_t. */
    template <typename func>
    void
    for_each_reverse (func op) const
    {
        for (int i (data_.size() - 1); i >= 0; --i)
        {
            for (auto& elem : data_[i])
                op(elem);
        }
    }

private:
    size_t
    index (chunk_coordinates pos) const
    {
        auto result (manhattan_distance(pos, pos_));
        assert(result < data_.size());
        return result;
    }

private:
    /** The current view distance, measured in chunks. */
    size_t              view_dist_;
    /** view_dist_ squared. */
    size_t              sq_view_dist_;
    /** The position of the center chunk. */
    chunk_coordinates   pos_;
    /** The actual data. */
    sorted_t            data_;
};

} // namespace hexa


