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
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <array>
#include <memory>
#include <boost/thread/mutex.hpp>
#include "block_types.hpp"
#include "chunk_base.hpp"
#include "serialize.hpp"

namespace hexa {

/** A single block in the game world. */
struct block
{
    /** The material ID. \sa material() \sa material_prop */
    uint16_t    type;

    /** Construct a block of a given type. */
    block (uint16_t t = type::air) : type (t) {}

    operator uint16_t () { return type; }

    /** Check if another block is of the same type. */
    bool operator== (block compare) const
        { return type == compare.type; }

    bool operator== (uint16_t compare) const
        { return type == compare; }

    bool operator< (block compare) const
        { return type < compare.type; }

    template <class archive>
    archive& serialize(archive& ar)
        { return ar(type); }

    bool is_air() const { return type == 0; }
};

/** A cube-shaped section of the game world's terrain, usually 16x16x16.
 *  The size of the cube is defined by \a chunk_size.  The game world is
 *  basically a sparse 3-D array of such chunks. Refer to \a chunk_base
 *  for an example of how to use this class. */
class chunk : public chunk_base<block>
{
    typedef chunk_base<block>   base;

public:
    /** The time at which this chunk was last used.
     *  This time stamp is used to determine which chunks need to be
     *  flushed from memory first. */
    gameclock_t     last_used;

    /** How far along this chunk is in the generation process.
     *  This is only used server side.  If the terrain contains features that
     *  span several chunks, this may leave some chunks around the edge in
     *  a half finished state.  This variable keeps track of this. */
    uint8_t         generation_phase;

private:
    /** Mutex for multithreaded terrain generation. */
    std::unique_ptr<boost::mutex>  lock_;

public:
    chunk()
        : last_used(0)
        , generation_phase(0)
        , lock_(new boost::mutex)
    { }

    chunk(chunk&& move) noexcept
        : base (std::move(move))
        , last_used (move.last_used)
        , generation_phase (move.generation_phase)
        , lock_ (std::move(move.lock_))
    { }

    boost::mutex& lock() { return *lock_; }
    const boost::mutex& lock() const { return *lock_; }

    /** */
    bool    operator== (const chunk& compare) const
        { return base::operator==(compare); }

    template <class archive>
    archive& serialize(archive& ar)
    {
        return ar.raw_data(*this, chunk_volume)(last_used)(generation_phase);
    }
};

/** Reference-counted pointer to a chunk. */
typedef std::shared_ptr<chunk>        chunk_ptr;


} // namespace hexa

namespace std {

inline
ostream& operator<< (ostream& str, hexa::block b)
{
    return str << b.type;
}

} // namespace std

