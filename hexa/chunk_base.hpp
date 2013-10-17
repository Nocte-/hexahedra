//---------------------------------------------------------------------------
/// \file   hexa/chunk_base.hpp
/// \brief  A 16x16x16 group of voxels.
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
#include <cassert>
#include <vector>
#include <boost/utility.hpp>

#include "basic_types.hpp"

namespace hexa {

/** A group of blocks.
 * For performance reasons, the world is generated, swapped to disk,
 * and sent to the client per chunk, instead of by block.
 *
 * Example code:
 * \code
 *
 *  chunk_base<int> example;
 *  chunk_index pos (3, 4, 6);
 *  example[pos] = 24;
 *
 *  chunk_index offset (0, 0, 1);
 *  example[pos + offset] = 25;
 *
 * \endcode */
template <class type, size_t dim=chunk_size>
class chunk_base : private std::vector<type>, boost::noncopyable
{
    typedef std::vector<type>      array_t;

public:
    typedef typename array_t::value_type        value_type;
    typedef typename array_t::size_type         size_type;
    typedef typename array_t::iterator          iterator;
    typedef typename array_t::const_iterator    const_iterator;

    using array_t::operator[];
    using array_t::size;
    using array_t::begin;
    using array_t::end;

public:
    /** If this flag is set, the storage backends know they're dealing
     * with a chunk that has been modified in memory. */
    bool    is_dirty;

public:
    chunk_base()
        : is_dirty (true)
    {
        array_t::resize(volume());
    }

    chunk_base(chunk_base&& move)
        : array_t   (move)
        , is_dirty  (move.is_dirty)
    {
        move.is_dirty = false;
        assert(size() == volume());
    }

    /** Fill this chunk with zeroes/air. */
    void clear(value_type zero)
    {
        std::fill(begin(), end(), zero);
        is_dirty = true;
    }

    /** Indexing operator.
     * \param idx The position of the element */
    inline
    value_type&     operator[](chunk_index idx)
        { return operator()(idx.x, idx.y, idx.z); }

    /** Indexing operator.
     * \param idx The position of the element */
    inline
    value_type      operator[](chunk_index idx) const
        { return operator()(idx.x, idx.y, idx.z); }

    /** Indexing operator. */
    inline
    value_type&     operator()(uint8_t x, uint8_t y, uint8_t z)
    {
        assert(size() == volume());
        assert (x < length());
        assert (y < length());
        assert (z < length());

        return array_t::operator[](x + y * length() + z * area());
    }

    /** Indexing operator. */
    inline
    value_type      operator()(uint8_t x, uint8_t y, uint8_t z) const
    {
        assert(size() == volume());
        assert (x < length());
        assert (y < length());
        assert (z < length());

        return array_t::operator[](x + y * length() + z * area());
    }

    bool operator== (const chunk_base<type>& compare) const
        { return std::equal(begin(), end(), compare.begin()); }

    /** Dummy resize function.
     * This function only exists so this class plays nice with some
     * algorithms that expect "resize" to exist.  It's okay to call it,
     * just as long as you don't try to resize it to anything but its
     * original volume. */
    void resize(size_t dummy)
    {
        assert(size() == volume());
        assert(dummy == volume());
    }

    /** Check if a chunk is empty.
     * This is "empty" in the STL sense; since air also has a value, a
     * chunk is never truly empty, and this function always returns false */
    constexpr bool empty() const { return false; }

    /** Get the length of this chunk (usually 16). */
    constexpr size_t length() const { return dim; }

    /** Get the area of this chunk's faces (usually 256). */
    constexpr size_t area() const   { return dim * dim; }

    /** Get the volume of this chunk (usually 4096). */
    constexpr size_t volume() const { return dim * dim * dim; }

    /** Convert an array index to a coordinate index.
     * Chunks are conceptually a cube, but in memory they're a flat array.
     * This function converts between the two. */
    chunk_index index_to_pos(size_type i) const
    {
        return chunk_index(i % dim, (i / dim) % dim, (i / (dim*dim)) % dim);
    }

    /** Convert an iterator to a coordinate index.
     * Chunks are conceptually a cube, but in memory they're a flat array.
     * This function converts between the two. */
    chunk_index index_to_pos(const_iterator i) const
    {
        return index_to_pos(size_type(std::distance(begin(), i)));
    }

    bool is_air() const
    {
        for (auto& blk : *this)
        {
            if (blk.type != 0)
                return false;
        }
        return true;
    }
};

} // namespace hexa

