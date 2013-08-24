//---------------------------------------------------------------------------
/// \file   hexa/compression.hpp
/// \brief  Convenience functions and classes for using LZ4 compression
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

#include "lz4/lz4.h"
#include "basic_types.hpp"
#include "serialize.hpp"

namespace hexa {

/** A buffer holding compressed data. */
class compressed_data
{
    typedef std::vector<char> buf_t;

public:
    /** The buffer with the compressed data. */
    buf_t       buf;
    /** The length of the uncompressed data. */
    uint16_t    unpacked_len;

public:
    typedef buf_t::iterator         iterator;
    typedef buf_t::const_iterator   const_iterator;

public:
    compressed_data() : unpacked_len(0) { }

    compressed_data(compressed_data&&) = default;

    compressed_data& operator= (compressed_data&& m)
    {
        if (&m != this)
        {
            buf = std::move(m.buf);
            unpacked_len = m.unpacked_len;
        }
        return *this;
    }

    void resize(size_t len)
    {
        buf.resize(len);
    }

    bool empty() const
    {
        return buf.empty();
    }

    char*       ptr()       { assert(!buf.empty()) ; return &buf[0]; }
    const char* ptr() const { assert(!buf.empty()) ; return &buf[0]; }

    size_t         size() const  { return buf.size(); }
    iterator       begin()       { return buf.begin(); }
    const_iterator begin() const { return buf.begin(); }
    iterator       end()         { return buf.end(); }
    const_iterator end() const   { return buf.end(); }

    bool operator== (const compressed_data& compare) const
    {
        return    unpacked_len == compare.unpacked_len
               && buf          == compare.buf;
    }

    template <class archive>
    archive& serialize(archive& ar) const
    {
        return ar(unpacked_len)(buf);
    }

    template <class archive>
    archive& serialize(archive& ar)
    {
        return ar(unpacked_len)(buf);
    }
};


/** Compress a buffer
 * \param in   The data to be compressed.  Note that this buffer must be
 *             smaller than 65536 bytes.
 * \return The compressed data.  */
template <class input_t>
compressed_data compress (const input_t& in)
{
    size_t byte_size (in.size() * sizeof(typename input_t::value_type));
    if (byte_size > 0xffff)
        throw std::runtime_error("too much data for compression");

    auto in_ptr (reinterpret_cast<const char*>(&*in.begin()));
    compressed_data out;
    if (byte_size > 0)
    {
        out.resize(byte_size + 16);
        std::fill(out.begin(), out.end(), 0);
        out.unpacked_len = byte_size;

        int compressed_length (LZ4_compress(in_ptr, out.ptr(), byte_size));
        if (compressed_length <= 0)
            throw std::runtime_error("lz4 compression failed");

        out.resize(compressed_length);
    }

    return out;
}


/** Decompress a buffer.
 * \param in    The compressed data
 * \param out   Where to put the decompressed data
 * \return \a out */
template <class output_t>
output_t& decompress (const compressed_data& in, output_t& out)
{
    uint16_t byte_size (in.unpacked_len);

    // Make room for the unpacked data
    out.resize(byte_size / sizeof(typename output_t::value_type));
    std::fill(out.begin(), out.end(), 0);
    if (byte_size > 0)
    {
        auto out_ptr (reinterpret_cast<char*>(&*out.begin()));
        int output_length (LZ4_uncompress(in.ptr(), out_ptr, byte_size));
        if (output_length < 0)
            throw std::runtime_error("lz4 decompression failed");

        assert((size_t)output_length == in.size());
    }

    return out;
}


template <class output_t>
output_t decompress_as (const compressed_data& in)
{
    output_t out;
    return decompress(in, out);
}


inline
binary_data decompress (const compressed_data& in)
{
    return decompress_as<binary_data>(in);
}

} // namespace hexa

