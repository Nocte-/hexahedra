//---------------------------------------------------------------------------
/// \file   serialize.hpp
/// \brief  (De)serialize plain old data types.
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

#include <array>
#include <cstdint>
#include <deque>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef WIN32
#  include <winsock2.h>
#else
#  include <sys/types.h>
#  include <netinet/in.h>
#endif

#include <boost/bind.hpp>

#include "aabb.hpp"
#include "vector2.hpp"
#include "vector3.hpp"
#include "wfpos.hpp"

// Turn off Clang++ warnings about the register keyword in the system headers
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated"
#endif

namespace hexa {

/// Convert a 64-bit integer from network to host order.
inline
uint64_t ntohll(uint64_t x)
{
    return ((uint64_t)(ntohl((uint32_t)((x << 32) >> 32))) << 32) | (uint32_t)ntohl(((uint32_t)(x >> 32)));
}

/// Convert a 64-bit integer from host to network order.
inline
uint64_t htonll(uint64_t x)
{
    return ntohll(x);
}

/// Serializes common data types to a binary representation
template <class obj>
class serializer
{
    obj&                              write_; ///< Binary data

    typedef serializer<obj>           self;
    typedef typename obj::value_type  value_type;
    typedef typename obj::size_type   size_type;

    /// This union is used to convert floats to integers.
    typedef union
    {
        uint32_t    integer;
        float       real;
    }
    conversion;

    /// This union is used to convert doubles to integers.
    typedef union
    {
        uint64_t    integer;
        double      real;
    }
    conversion_dbl;

public:
    serializer(obj& o) : write_ (o) {}

    template <class t>
    void write (const t val)
    {
        size_type pos (write_.size());
        write_.resize(pos + sizeof(t));
        const value_type* ptr = reinterpret_cast<const value_type*>(&val);
        std::copy(ptr, ptr + sizeof(t), write_.begin() + pos);
    }

    self& operator() (const bool val)
    {
        write_.push_back(uint8_t(val ? 1 : 0));
        return *this;
    }

    self& operator() (const char val)
    {
        write_.push_back(val);
        return *this;
    }

    self& operator() (const uint8_t val)
    {
        write_.push_back(val);
        return *this;
    }

    self& operator() (const int8_t val)
    {
        write_.push_back(val);
        return *this;
    }

    self& operator() (const direction_type val)
    {
        return operator()(static_cast<uint8_t>(val));
    }

    self& operator() (const uint16_t val)
    {
        write(htons(val));
        return *this;
    }

    self& operator() (const int16_t val)
    {
        write(htons(val));
        return *this;
    }

    self& operator() (const uint32_t val)
    {
        write(htonl(val));
        return *this;
    }

    self& operator() (const int32_t val)
    {
        write(htonl(val));
        return *this;
    }

    self& operator() (const float val)
    {
        conversion c;
        c.real = val;
        write(htonl(c.integer));
        return *this;
    }

    self& operator() (const uint64_t val)
    {
        write(htonll(val));
        return *this;
    }

    self& operator() (const double val)
    {
        conversion_dbl c;
        c.real = val;
        write(htonll(c.integer));
        return *this;
    }

    self& operator() (const std::basic_string<char>& val)
    {
        if (val.size() * sizeof(char) > 65535)
            throw std::runtime_error("string too long");

        uint16_t byte_size (val.size() * sizeof(char));
        write(htons(byte_size));
        if (byte_size > 0)
        {
            size_type pos(write_.size());
            write_.resize(pos + byte_size);
            const char* ptr(reinterpret_cast<const char*>(&*val.begin()));
            std::copy(ptr, ptr + byte_size, write_.begin() + pos);
        }
        return *this;
    }

    self& operator() (const std::vector<char>& val)
    {
        if (val.size() > 65535)
            throw std::runtime_error("array too long");

        uint16_t byte_size (val.size());
        write(htons(byte_size));
        write_.insert(write_.end(), val.begin(), val.end());

        return *this;
    }

    template <class t>
    self& operator() (const std::vector<t>& val)
    {
        uint16_t array_size (val.size());
        write(htons(array_size));
        for (uint16_t i (0); i < array_size; ++i)
             (*this)(val[i]);

        return *this;
    }

    template <typename t, size_t count>
    self& operator() (const std::array<t, count>& val)
    {
        for (size_t i (0); i < count; ++i)
            (*this)(val[i]);

        return *this;
    }

    template <class t>
    self& operator() (const vector2<t>& val)
    {
        return (*this)(val.x)(val.y);
    }

    self& operator() (const vector3<int8_t>& val)
    {
        return (*this)(uint16_t(val.x + chunk_size * val.y + chunk_area * val.z));
    }

    template <class t>
    self& operator() (const vector3<t>& val)
    {
        return (*this)(val.x)(val.y)(val.z);
    }

    self& operator() (const wfpos& val)
    {
        return (*this)(val.pos)(val.frac);
    }

    template <typename t>
    self& operator() (const aabb<t>& val)
    {
        return (*this)(val.first)(val.second);
    }

    template <class t>
    self& operator() (const t& val)
    {
        t& ncval (*const_cast<t*>(&val));
        ncval.serialize(*this);
        return *this;
    }

    template <class t>
    self& raw_data(const t& val, size_t elements)
    {
        // The 'elements' parameter seems useless here, but it's used in
        // the deserializer.
        //
        assert(val.size() == elements);
        if (!val.empty())
        {
            const char* s_ptr (reinterpret_cast<const char*>(&*val.begin()));
            const char* e_ptr(s_ptr + sizeof(typename t::value_type) * val.size());
            write_.insert(write_.end(), s_ptr, e_ptr);
        }

        return *this;
    }
};

/// Create a serializer.
//  This is just a convenience function to avoid having to specify the
//  destination's type in a template.
// @param dest      Data will be serialized to this object
// @return A serializer object for \a dest
template <class obj>
serializer<obj> make_serializer (obj& dest)
{
    return serializer<obj>(dest);
}

/// Serialize a single object.
template <class obj>
binary_data serialize (obj& o)
{
    binary_data buffer;
    make_serializer(buffer)(o);
    return buffer;
}

template <class obj>
binary_data serialize_c (obj o)
{
    binary_data buffer;
    make_serializer(buffer)(o);
    return buffer;
}

//---------------------------------------------------------------------------

/// Deserializes common data types from a binary representation
template <typename obj>
class deserializer
{
    //const obj&  read_; ///< The binary representation

    typedef deserializer<obj>   self;
    typedef const char*         ptr_t;

    ptr_t   first_;
    ptr_t   last_;
    ptr_t   cursor_;

    /// This union is used to convert integers to floats.
    typedef union
    {
        uint32_t    integer;
        float       real;
    }
    conversion;

    inline void boundary_check(int size)
    {
        assert(std::distance(cursor_, last_) >= size);
    }

public:
    deserializer (const obj& o)
        : first_  (reinterpret_cast<ptr_t>(&*o.begin()))
        , last_   (reinterpret_cast<ptr_t>(&*o.end()))
        , cursor_ (first_)
    { }

    template <typename iter>
    deserializer(iter first, iter last)
        : first_    (reinterpret_cast<ptr_t>(&*first))
        , last_     (reinterpret_cast<ptr_t>(&*last))
        , cursor_   (first_)
    { }

    size_t bytes_read() const
        { return std::distance(first_, cursor_); }

    size_t bytes_left() const
        { return std::distance(cursor_, last_); }

    self& operator() (bool& val)
    {
        boundary_check(1);
        val = ((*cursor_++) != 0);
        return *this;
    }

    self& operator() (char& val)
    {
        boundary_check(1);
        val = *cursor_++;
        return *this;
    }

    self& operator() (signed char& val) // Looks dumb, but it is needed.
    {
        boundary_check(1);
        val = *cursor_++;
        return *this;
    }

    self& operator() (unsigned char& val)
    {
        boundary_check(1);
        val = *cursor_++;
        return *this;
    }

    self& operator() (direction_type& val)
    {
        boundary_check(1);
        val = static_cast<direction_type>(*cursor_++);
        return *this;
    }

    self& operator() (uint16_t& val)
    {
        boundary_check(2);
        val = ntohs(*reinterpret_cast<const uint16_t*>(cursor_));
        std::advance(cursor_, 2);
        return *this;
    }

    self& operator() (int16_t& val)
    {
        boundary_check(2);
        val = ntohs(*reinterpret_cast<const int16_t*>(cursor_));
        std::advance(cursor_, 2);
        return *this;
    }

    self& operator() (uint32_t& val)
    {
        boundary_check(4);
        val = ntohl(*reinterpret_cast<const uint32_t*>(cursor_));
        std::advance(cursor_, 4);
        return *this;
    }

    self& operator() (int32_t& val)
    {
        boundary_check(4);
        val = ntohl(*reinterpret_cast<const int32_t*>(cursor_));
        std::advance(cursor_, 4);
        return *this;
    }

    self& operator() (float& val)
    {
        boundary_check(4);
        conversion c;
        c.integer = ntohl(*reinterpret_cast<const uint32_t*>(cursor_));
        val = c.real;
        std::advance(cursor_, 4);
        return *this;
    }

    self& operator() (uint64_t& val)
    {
        boundary_check(8);
        val = ntohll(*reinterpret_cast<const uint64_t*>(cursor_));
        std::advance(cursor_, 8);
        return *this;
    }

    self& operator() (double& val)
    {
        boundary_check(8);
        uint64_t temp (ntohll(*reinterpret_cast<const uint64_t*>(cursor_)));
        val = *reinterpret_cast<double*>(temp);
        std::advance(cursor_, 8);
        return *this;
    }

    self& operator() (std::string& val)
    {
        uint16_t len;
        (*this)(len);
        if (len == 0)
            return *this;

        if (bytes_left() < len)
        {
            assert(false);
            throw std::runtime_error("end of string reached");
        }

        val.resize(len);
        std::copy(cursor_, cursor_ + len, val.begin());
        std::advance(cursor_, len);

        return *this;
    }

    self& operator() (std::vector<char>& val)
    {
        uint16_t len;
        (*this)(len);
        if (len == 0)
            return *this;

        if (bytes_left() < len)
            throw std::runtime_error("end of array reached");

        val.resize(len);
        std::copy(cursor_, cursor_ + len, &*val.begin());
        std::advance(cursor_, len);

        return *this;
    }

    template <class t>
    self& operator() (std::vector<t>& val)
    {
        uint16_t len;
        (*this)(len);

        val.resize(len);
        for(uint16_t i (0); i < len; ++i)
            (*this)(val[i]);

        return *this;
    }

    template <typename t, size_t count>
    self& operator() (std::array<t, count>& val)
    {
        boundary_check(count * sizeof(t));
        for (size_t i (0); i < count; ++i)
            (*this)(val[i]);

        return *this;
    }

    template <class t>
    self& operator() (vector2<t>& val)
    {
        boundary_check(2 * sizeof(t));
        return (*this)(val.x)(val.y);
    }

    self& operator() (vector3<int8_t>& val)
    {
        boundary_check(2);

        // Offsets within a chunk are stored in a more compact form
        uint16_t temp;
        (*this)(temp);
        val.x = temp % chunk_size;
        val.y = (temp / chunk_size) % chunk_size;
        val.z = (temp / chunk_area) % chunk_size;
        return *this;
    }

    template <class t>
    self& operator() (vector3<t>& val)
    {
        boundary_check(3 * sizeof(t));
        return (*this)(val.x)(val.y)(val.z);
    }

    self& operator() (wfpos& val)
    {
        return (*this)(val.pos)(val.frac);
    }

    template <typename t>
    self& operator() (aabb<t>& val)
    {
        return (*this)(val.first)(val.second);
    }

    template <class t>
    self& operator() (t& val)
    {
        return val.serialize(*this);
    }

    template <class t>
    self& raw_data(t& val, size_t elements)
    {
        if (elements == 0)
            return *this;

        size_t bytes(elements * sizeof(typename t::value_type));
        if (bytes_left() < bytes)
            throw std::runtime_error("end of array reached");

        val.resize(elements);
        char* ptr (reinterpret_cast<char*>(&*val.begin()));
        std::copy(cursor_, cursor_ + bytes, ptr);
        std::advance(cursor_, bytes);

        return *this;
    }

    template <typename t>
    t get()
    {
        t tmp;
        operator()(tmp);
        return tmp;
    }

    template <typename t>
    std::vector<t> get(size_t elements)
    {
        std::vector<t> tmp;
        raw_data(tmp, elements);
        return tmp;
    }
};

/// Create a deserializer.
//  This is just a convenience function to avoid having to specify the
//  source's type in a template.
// @param src       Data will be deserialized from this object
// @return A deserializer object for \a dest
template <typename obj>
deserializer<obj>
make_deserializer (const obj& src)
{
    return deserializer<obj>(src);
}

template <typename iter_t>
deserializer<std::vector<char>>
make_deserializer (iter_t first, iter_t last)
{
    return deserializer<std::vector<char>>(first, last);
}

template <typename obj, typename buf_t>
obj deserialize_as (const buf_t& buffer, obj result = obj())
{
    if (!buffer.empty())
        make_deserializer(buffer)(result);

    return result;
}

} // namespace hexa

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#endif
