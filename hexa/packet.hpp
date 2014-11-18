//---------------------------------------------------------------------------
/// \file   packet.hpp
/// \brief  TCP or UDP network packet
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
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <vector>

#include "crypto.hpp"

namespace hexa
{

/** A network packet. */
class packet
{
public:
    typedef uint8_t value_type;
    typedef value_type* iterator;
    typedef const value_type* const_iterator;
    typedef size_t size_type;

public:
    packet(uint8_t* buf, size_type len)
        : buf_{buf}
        , size_{len}
    {
    }

    // packet(std::vector<char>&& buffer) {}

    packet(const packet&) = default;

    /** Get the message type from the body.
     * @pre The body is not empty
     * @return The message type */
    uint8_t message_type() const { return *buf_; }

    bool empty() const { return size_ == 0; }
    size_type size() const { return size_; }

    iterator raw_data() { return buf_; }
    iterator body() { return buf_ + 1; }

    iterator begin() { return buf_ + 1; }
    iterator end() { return buf_ + size_; }

    const_iterator begin() const { return buf_ + 1; }
    const_iterator end() const { return buf_ + size_; }

    std::string dump() const
    {
        std::stringstream s;
        s << "Packet type: " << (uint16_t)message_type() << std::endl;
        s << "Data:" << std::hex << std::fixed << std::setw(2);
        for (auto x : *this)
            s << " " << (uint16_t)x;

        return s.str();
    }

    bool is_encrypted() const { return size() > 5 && *buf_ == 0xff; }

    void decrypt(const crypto::buffer& iv, crypto::aes& cr)
    {
        assert(size() > 5);
        assert(*buf_ == 0xff); // Encrypted packets always start with 0xff
        auto counted_iv = iv;
        // Bytes 1-4 are the 32-bit game timer. XOR it with the IV we got from
        // the server to make the CTR IV.
        uint32_t timer = *(reinterpret_cast<uint32_t*>(buf_ + 1));
        *(reinterpret_cast<uint32_t*>(&counted_iv[0])) ^= timer;
        // Decrypt everything after the 5 bytes of the header, and pretend
        // it's a normal packet.
        buf_ += 5;
        size_ -= 5;
        cr.decrypt(counted_iv, buf_, size(), buf_);
    }

private:
    value_type* buf_;
    size_type size_;
};

} // namespace hexa
