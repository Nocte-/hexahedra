//---------------------------------------------------------------------------
/// \file   area_data.hpp
/// \brief  A flat, 16x16 part of the world's map.
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
// Copyright 2013-2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include <cassert>
#include <vector>

#include "array.hpp"
#include "basic_types.hpp"
#include "serialize.hpp"

namespace hexa
{

/** A flat, 16x16 part of the world's map.
 *  It is used to store things like a detailed height map, or different
 *  kinds of biome info such as temperature or humidity. */
class area_data //: public array_2d<int16_t, chunk_size, chunk_size>
{
    typedef std::vector<int16_t> array_t;
    array_t buf_;

public:
    typedef array_t::value_type value_type;
    typedef array_t::iterator iterator;
    typedef array_t::const_iterator const_iterator;

public:
    area_data() { buf_.resize(chunk_area); }

    area_data(const area_data&) = default;
    area_data& operator=(const area_data&) = default;

#ifdef _MSC_VER
    area_data(area_data&& m)
        : buf_(std::move(m.buf_))
    {
    }

    area_data& operator=(area_data&& m)
    {
        if (this != &m)
            buf_ = std::move(m.buf_);

        return *this;
    }
#else
    area_data(area_data&&) = default;
    area_data& operator=(area_data&&) = default;
#endif

    area_data(std::vector<int16_t>&& m)
        : buf_(std::move(m))
    {
    }

    /** Fill the area with zeroes (or a given value). */
    void clear(value_type v = 0) { std::fill(begin(), end(), v); }

    /** Copy the contents to another area. */
    void copy(const area_data& other) { buf_ = other.buf_; }

    iterator begin() { return buf_.begin(); }
    const_iterator begin() const { return buf_.begin(); }
    iterator end() { return buf_.end(); }
    const_iterator end() const { return buf_.end(); }

    value_type& operator[](map_index pos) { return operator()(pos.x, pos.y); }

    value_type operator[](map_index pos) const
    {
        return operator()(pos.x, pos.y);
    }

    value_type& operator()(uint8_t x, uint8_t y)
    {
        assert(x < chunk_size);
        assert(y < chunk_size);
        return buf_[x + y * chunk_size];
    }

    value_type operator()(uint8_t x, uint8_t y) const
    {
        assert(x < chunk_size);
        assert(y < chunk_size);
        return buf_[x + y * chunk_size];
    }

    value_type get(int x, int y) { return operator()(x, y); }

    void set(int x, int y, int value) { operator()(x, y) = value; }

    /** Dummy resize function.
     *  This function only exists to make this class play nice with other
     *  algorithms that expect the 'resize' function to exist. */
    void resize(size_t dummy) { assert(dummy == chunk_area); }

    /** Return the number of elements (usually 256). */
    size_t size() const { return chunk_area; }

    bool empty() const { return false; }

public:
    template <typename Archiver>
    Archiver& serialize(Archiver& ar)
    {
        return ar.raw_data(*this, size());
    }
};

} // namespace hexa
