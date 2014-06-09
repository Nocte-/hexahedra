//---------------------------------------------------------------------------
/// \file   hexa/world_subsection.hpp
/// \brief  A subsection of the game world.
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

#include "aabb.hpp"
#include "algorithm.hpp"
#include "basic_types.hpp"
#include "trace.hpp"
#include "voxel_range.hpp"

namespace hexa
{

template <typename chunk_ptr_type>
class world_subsection
{
    typedef std::unordered_map<chunk_coordinates, chunk_ptr_type> storage_t;

public:
    typedef chunk_ptr_type chunk_pointer_type;
    typedef typename chunk_ptr_type::element_type chunk_type;
    typedef typename chunk_type::value_type value_type;
    typedef typename chunk_type::size_type size_type;

    typedef typename storage_t::iterator iterator;
    typedef typename storage_t::const_iterator const_iterator;

public:
    world_subsection() {}

    world_subsection(
        std::unordered_map<chunk_coordinates, chunk_ptr_type>&& init)
        : cache_{std::move(init)}
    {
    }

    world_subsection(world_subsection&& m)
        : cache_{std::move(m.cache_)}
    {
    }

public:
    bool has_chunk(chunk_coordinates pos) const
    {
        return cache_.count(pos) > 0;
    }

    const chunk_type& get_chunk(chunk_coordinates pos) const
    {
        return *lookup(cache_, pos);
    }

    chunk_type& get_chunk(chunk_coordinates pos)
    {
        return *lookup(cache_, pos);
    }

    chunk_pointer_type& get_ptr(chunk_coordinates pos)
    {
        return lookup(cache_, pos);
    }

    void set_chunk(chunk_coordinates pos, const chunk_pointer_type& ptr)
    {
        if (ptr != nullptr)
            cache_[pos] = ptr;
        else
            cache_.erase(pos);
    }

    value_type& operator[](world_coordinates o)
    {
        return get_chunk(o >> cnkshift)[o % chunk_size];
    }

    value_type& operator()(uint32_t x, uint32_t y, uint32_t z)
    {
        return operator[](world_coordinates(x, y, z));
    }

    const value_type& operator[](world_coordinates o) const
    {
        return get_chunk(o >> cnkshift)[o % chunk_size];
    }

    const value_type& operator()(uint32_t x, uint32_t y, uint32_t z) const
    {
        return operator[](world_coordinates(x, y, z));
    }

public:
    size_type size() const { return cache_.size(); }

    bool empty() const { return cache_.empty(); }

    iterator begin() { return cache_.begin(); }
    const_iterator begin() const { return cache_.begin(); }
    const_iterator cbegin() const { return cache_.begin(); }
    iterator end() { return cache_.end(); }
    const_iterator end() const { return cache_.end(); }
    const_iterator cend() const { return cache_.end(); }

protected:
    storage_t cache_;
};

} // namespace hexa
