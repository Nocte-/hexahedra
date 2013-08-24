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

#include "vector2.hpp"
#include "vector3.hpp"
#include "wfpos.hpp"

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
    void write (t val)
    {
        size_type pos (write_.size());
        write_.resize(pos + sizeof(t));
        const value_type* ptr = reinterpret_cast<const value_type*>(&val);
        std::copy(ptr, ptr + sizeof(t), write_.begin() + pos);
    }

    self& operator() (bool val)
    {
        write_.push_back(uint8_t(val ? 1 : 0));
        return *this;
    }

    self& operator() (char val)
    {
        write_.push_back(val);
        return *this;
    }

    self& operator() (uint8_t val)
    {
        write_.push_back(val);
        return *this;
    }

    self& operator() (int8_t val)
    {
        write_.push_back(val);
        return *this;
    }

    self& operator() (direction_type val)
    {
        return operator()(static_cast<uint8_t>(val));
    }

    self& operator() (uint16_t val)
    {
        write(htons(val));
        return *this;
    }

    self& operator() (int16_t val)
    {
        write(htons(val));
        return *this;
    }

    self& operator() (uint32_t val)
    {
        write(htonl(val));
        return *this;
    }

    self& operator() (int32_t val)
    {
        write(htonl(val));
        return *this;
    }

    self& operator() (float val)
    {
        conversion c;
        c.real = val;
        write(htonl(c.integer));
        return *this;
    }

    self& operator() (uint64_t val)
    {
        write(htonll(val));
        return *this;
    }

    self& operator() (double val)
    {
        conversion_dbl c;
        c.real = val;
        write(htonll(c.integer));
        return *this;
    }

    template <class char_t>
    self& operator() (std::basic_string<char_t>& val)
    {
        if (val.size() * sizeof(char_t) > 65535)
            throw std::runtime_error("string too long");

        uint16_t byte_size (val.size() * sizeof(char));
        write(htons(byte_size));
        size_type pos (write_.size());
        write_.resize(pos + byte_size);
        const char* ptr (reinterpret_cast<const char*>(&*val.begin()));
        std::copy(ptr, ptr + byte_size, write_.begin() + pos);

        return *this;
    }

    self& operator() (std::vector<char>& val)
    {
        if (val.size() > 65535)
            throw std::runtime_error("array too long");

        uint16_t byte_size (val.size());
        write(htons(byte_size));
        write_.insert(write_.end(), val.begin(), val.end());

        return *this;
    }

    template <class t>
    self& operator() (std::vector<t>& val)
    {
        uint16_t array_size (val.size());
        write(htons(array_size));
        for (uint16_t i (0); i < array_size; ++i)
             (*this)(val[i]);

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

    template <class t>
    self& operator() (vector2<t>& val)
    {
        return (*this)(val.x)(val.y);
    }

    self& operator() (vector3<int8_t>& val)
    {
        return (*this)(uint16_t(val.x + chunk_size * val.y + chunk_area * val.z));
    }

    template <class t>
    self& operator() (const vector3<t>& val)
    {
        return (*this)(val.x)(val.y)(val.z);
    }

    template <class t>
    self& operator() (vector3<t>& val)
    {
        return (*this)(val.x)(val.y)(val.z);
    }

    self& operator() (wfpos& val)
    {
        return (*this)(val.pos)(val.frac);
    }

    self& operator() (const wfpos& val)
    {
        return (*this)(val.pos)(val.frac);
    }

    template <class t>
    self& operator() (t& val)
    {
        val.serialize(*this);
        return *this;
    }

    template <class t>
    self& raw_data(t& val, size_t elements)
    {
        // The 'elements' parameter seems useless here, but it's used in
        // the deserializer.
        //
        assert(val.size() == elements);
        if (!val.empty())
        {
            const char* s_ptr (reinterpret_cast<const char*>(&*val.begin()));
            const char* e_ptr (reinterpret_cast<const char*>(&*val.end()));
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
template <class obj>
class deserializer
{
    const obj&  read_; ///< The binary representation

    typedef deserializer<obj>       self;
    typename obj::const_iterator    cursor_;


    /// This union is used to convert integers to floats.
    typedef union
    {
        uint32_t    integer;
        float       real;
    }
    conversion;

public:
    deserializer(const obj& o) : read_ (o), cursor_ (o.begin()) { }

    size_t bytes_read() const
        { return std::distance(read_.begin(), cursor_); }

    size_t bytes_left() const
        { return std::distance(cursor_, read_.end()); }

    self& operator() (bool& val)
    {
        val = (*cursor_++) != 0;
        return *this;
    }

    self& operator() (char& val)
    {
        val = *cursor_++;
        return *this;
    }

    self& operator() (signed char& val) // Looks dumb, but it is needed.
    {
        val = *cursor_++;
        return *this;
    }

    self& operator() (unsigned char& val)
    {
        val = *cursor_++;
        return *this;
    }

    self& operator() (direction_type& val)
    {
        val = static_cast<direction_type>(*cursor_++);
        return *this;
    }

    self& operator() (uint16_t& val)
    {
        val = ntohs(*reinterpret_cast<const uint16_t*>(&*cursor_));
        std::advance(cursor_, 2);
        return *this;
    }

    self& operator() (int16_t& val)
    {
        val = ntohs(*reinterpret_cast<const int16_t*>(&*cursor_));
        std::advance(cursor_, 2);
        return *this;
    }

    self& operator() (uint32_t& val)
    {
        val = ntohl(*reinterpret_cast<const uint32_t*>(&*cursor_));
        std::advance(cursor_, 4);
        return *this;
    }

    self& operator() (int32_t& val)
    {
        val = ntohl(*reinterpret_cast<const int32_t*>(&*cursor_));
        std::advance(cursor_, 4);
        return *this;
    }

    self& operator() (float& val)
    {
        conversion c;
        c.integer = ntohl(*reinterpret_cast<const uint32_t*>(&*cursor_));
        val = c.real;
        std::advance(cursor_, 4);
        return *this;
    }

    self& operator() (uint64_t& val)
    {
        val = ntohll(*reinterpret_cast<const uint64_t*>(&*cursor_));
        std::advance(cursor_, 8);
        return *this;
    }

    self& operator() (double& val)
    {
        uint64_t temp (ntohll(*reinterpret_cast<const uint64_t*>(&*cursor_)));
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

        if (std::distance(cursor_, read_.end()) < len)
        {
            std::cout << "end of string " << len << " " << std::distance(cursor_, read_.end())
                      << " " << std::distance(read_.begin(), read_.end()) << std::endl;
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

        if ((unsigned)std::distance(cursor_, read_.end()) < len)
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

    template <class t>
    self& operator() (vector2<t>& val)
    {
        return (*this)(val.x)(val.y);
    }

    self& operator() (vector3<int8_t>& val)
    {
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
        return (*this)(val.x)(val.y)(val.z);
    }

    self& operator() (wfpos& val)
    {
        return (*this)(val.pos)(val.frac);
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
        if ((unsigned)std::distance(cursor_, read_.end()) < bytes)
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
template <class obj>
deserializer<obj> make_deserializer (const obj& src)
{
    return deserializer<obj>(src);
}

template <class obj>
obj deserialize_as (const std::vector<char>& buffer, obj result = obj())
{
    if (!buffer.empty())
        make_deserializer(buffer)(result);

    return result;
}

} // namespace hexa

