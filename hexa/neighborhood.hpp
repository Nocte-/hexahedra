//---------------------------------------------------------------------------
/// \file   neighborhood.hpp
/// \brief  Fast access to a chunk and its neighbors.
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
#include <boost/range/algorithm/for_each.hpp>
#include "basic_types.hpp"
#include "chunk.hpp"
#include "storage_i.hpp"
#include "voxel_range.hpp"

namespace hexa {

/** A Moore neighborhood consists of a chunk, and the chunks surrounding
 ** it in a cube of a given radius.
 *
 * There are a few situations where an algorithm needs access to the
 * immediate neighbors of a chunk.  A few examples include determining
 * the visibility of a block's face, calculating light maps, and checking
 * player actions.
 *
 * Accessing a single block in the world involves two steps; a lookup of
 * the correct chunk in the cache (and possibly fetching it from the
 * persistent storage), and then finding the block in the chunk.  The first
 * step is the slowest one, because it involves a lookup in a hash table or
 * a binary tree.  (Getting a block from a chunk is a simple array lookup.)
 * This class speeds things up by caching the surrounding chunks, and makes
 * it easier to access the individual blocks. */
template <class chunk_ptr_type>
class neighborhood
{
public:
    typedef chunk_ptr_type  chunk_pointer_type;
    typedef typename chunk_ptr_type::element_type::value_type value_type;

public:
    /** Initialize the Moore neighborhood.
     *  It will be a cube of size (radius * 2 + 1).
     * @param src       The chunk data source
     * @param center    The coordinates of the center chunk
     * @param radius    The radius of the neighborhood */
    neighborhood (storage_i& src, const chunk_coordinates& center,
                  size_t radius = 1)
        : src_      (src)
        , center_   (center)
        , len_      (radius * 2 + 1)
        , cache_    (len_ * len_ * len_)
    {
    }

    world_vector offset() const { return world_vector(radius(), radius(), radius()); }
    size_t radius() const { return (len_ - 1) / 2; }
    size_t length() const { return len_; }
    size_t area()   const { return length() * length(); }
    size_t volume() const { return length() * area(); }

    /** Add a chunk as one of the neighbors.
     * @param relative_position  The position wrt the central chunk
     * @param cnk                The neighboring chunk */
    void add (block_vector relative_position, chunk_ptr_type cnk)
    {
        block_vector a (relative_position + offset());
        cache_[a.x + a.y * length() + a.z * area()] = cnk;
    }

    /** Get a block from the cache.
     * The range (0,0,0) to (\a hexa::chunk_limits) indexes the central chunk.
     * @param pos   The position of the block
     * @return The block at \a pos */
    value_type& operator[] (world_vector pos)
    {
        world_coordinates o (pos + offset() * chunk_size);
        assert(o.x < len_ * chunk_size);
        assert(o.y < len_ * chunk_size);
        assert(o.z < len_ * chunk_size);
        chunk_coordinates a (o / chunk_size);
        return (*access(a.x + a.y * length() + a.z * area()))[o % chunk_size];
    }

    /** Get a block from the cache.
     * The range (0,0,0) to (\a hexa::chunk_limits) indexes the central chunk.
     * Anything else from (-\a hexa::chunk_limits) to
     * (2 * \a hexa::chunk_limits) indexes blocks from the surrounding chunks.
     * @param pos   The position of the block
     * @return The block at \a pos */
    const value_type& operator[] (world_vector pos) const
    {
        world_coordinates o (pos + offset() * chunk_size);
        assert(o.x < len_ * chunk_size);
        assert(o.y < len_ * chunk_size);
        assert(o.z < len_ * chunk_size);
        chunk_coordinates a (o / chunk_size);
        return (*access(a.x + a.y * length() + a.z * area()))[o % chunk_size];
    }

    /** Get the middle chunk. */
    chunk_ptr_type center() const { return access((cache_.size()-1)/2); }

    chunk_coordinates center_pos() const { return center_; }

    chunk_ptr_type chunk(world_vector pos) const
    {
        chunk_coordinates a (pos + offset());
        assert(a.x < len_);
        assert(a.y < len_);
        assert(a.z < len_);
        return access(a.x + a.y * length() + a.z * area());
    }

public:
    class iterator
    {
    public:
        typedef typename chunk_ptr_type::element_type::value_type value_type;
        typedef value_type&  reference;
        typedef size_t       difference_type;

        typedef std::forward_iterator_tag  iterator_category;

    public:
        iterator(neighborhood& n)
            : owner_(n), cell_(0), block_(0)
        {
            owner_.access(cell_);
        }

        iterator& end()
        {
            cell_ = owner_.cache_.size();
            block_ = 0;
            return *this;
        }

        block_vector pos() const
        {
            const size_t len (owner_.length());

            world_vector b (cell_ % len, (cell_ / len) % len, cell_ / (len*len));
            world_vector c (block_ % chunk_size, (block_ / chunk_size) % chunk_size, block_ / chunk_area);
            return  b * chunk_size + c - (world_vector(1,1,1) * chunk_size * owner_.radius());
        }

        iterator& operator++()
        {
            if (++block_ == chunk_volume)
            {
                block_ = 0;
                ++cell_;
                owner_.access(cell_);
            }
            return *this;
        }

        iterator operator++(int)
        {
            iterator temp (*this);
            operator++();
            return temp;
        }

        bool operator== (iterator compare) const
        {
            assert (&owner_ == &compare.owner_);
            return compare.cell_ == cell_ && compare.block_ == block_;
        }

        bool operator!= (iterator compare) const
        {
            return !operator==(compare);
        }

        reference operator*() const
        {
            return (*owner_.cache_[cell_])[block_];
        }

    private:
        neighborhood& owner_;
        uint16_t cell_;
        uint16_t block_;
    };

    friend class iterator;

    iterator begin()     { return iterator(*this); }
    iterator end()       { return iterator(*this).end(); }

protected:
    chunk_ptr_type access(size_t idx) const
    {
        assert(idx < cache_.size());

        if (cache_[idx] == nullptr)
        {
            world_vector b (idx % len_, (idx / len_) % len_, idx / (len_*len_));
            auto c (center_ + b - offset());
            auto height (src_.get_coarse_height(c));

            // Check for all-air blocks, or for undefined blocks
            if (   (height != undefined_height && c.z >= height)
                || (cache_[idx] = src_.get_chunk(c)).get() == nullptr)
            {
                cache_[idx] = empty_;
            }
            assert(cache_[idx] != nullptr);
        }

        return cache_[idx];
    }

protected:
    storage_i&                  src_;
    chunk_coordinates           center_;
    size_t                      len_;
    mutable std::vector<chunk_ptr_type> cache_;
    static chunk_ptr_type       empty_;
};


template <class t>
t neighborhood<t>::empty_ = t(new typename t::element_type);

} // namespace hexa

