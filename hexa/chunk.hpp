//---------------------------------------------------------------------------
/// \file   chunk.hpp
/// \brief  A chunk of the world's terrain.
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

#include <array>
#include <memory>
#include <boost/thread/mutex.hpp>
#include "block_types.hpp"
#include "chunk_base.hpp"
#include "compiler_fix.hpp"
#include "serialize.hpp"

namespace hexa
{

/** A single block in the game world. */
struct block
{
    /** The material ID. \sa material() \sa material_prop */
    uint16_t type;

    /** Construct a block of a given type. */
    block(uint16_t t = type::air)
        : type{t}
    {
    }

    operator uint16_t() { return type; }

    /** Check if another block is of the same type. */
    bool operator==(block compare) const { return type == compare.type; }

    bool operator==(uint16_t compare) const { return type == compare; }

    bool operator<(block compare) const { return type < compare.type; }

    template <typename Archive>
    Archive& serialize(Archive& ar)
    {
        return ar(type);
    }

    bool is_air() const { return type == 0; }
};

/** A cube-shaped section of the game world's terrain, usually 16x16x16.
 *  The size of the cube is defined by \a chunk_size.  The game world is
 *  basically a sparse 3-D array of such chunks. Refer to \a chunk_base
 *  for an example of how to use this class. */
class chunk : public chunk_base<block>
{
    typedef chunk_base<block> base;

public:
    /** Chunk version number.
     *  All chunks start at version 0. Every time a write operation
     *  finishes, the version number is increased.  Data that depends on
     *  the chunk status (surfaces, compressed chunks, etc.) use this
     *  number to check if they need to be updated.  It is also used to
     *  determine if new data needs to be sent to clients. */
    uint32_t version;

public:
    chunk()
        : version{0}
    {
    }

#ifdef _MSC_VER
    chunk(chunk&& m)
        : base(std::move(m))
    {
    }

    chunk(const chunk&) = default;

    chunk& operator=(chunk&& m)
    {
        if (this != &m) {
            version = m.version;
            base::operator=(std::move(m));
        }
        return *this;
    }

    chunk& operator=(const chunk&) = default;
#else

    chunk(chunk&&) = default;
    chunk(const chunk&) = default;
    chunk& operator=(chunk&&) = default;
    chunk& operator=(const chunk&) = default;

#endif

    bool operator==(const chunk& compare) const
    {
        return base::operator==(compare);
    }

    template <typename Archive>
    Archive& serialize(Archive& ar)
    {
        return ar.raw_data (*this, chunk_volume)(version);
    }
};

} // namespace hexa

//---------------------------------------------------------------------------

namespace std
{

inline ostream& operator<<(ostream& str, hexa::block b)
{
    return str << b.type;
}

inline ostream& operator<<(ostream& str, const hexa::chunk& cnk)
{
    for (int z = 0; z < 16; ++z) {
        str << "Z " << z << std::endl;
        for (int y = 0; y < 16; ++y) {
            str << y << ": ";
            for (int x = 0; x < 16; ++x)
                str << cnk(x, y, z) << " ";

            str << std::endl;
        }
        str << std::endl;
    }
    str << std::endl;

    return str;
}

} // namespace std
