//---------------------------------------------------------------------------
/// \file   server/world_write.hpp
/// \brief  Write access to the game world
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
// Copyright 2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>

#include <hexa/basic_types.hpp>
#include <hexa/chunk.hpp>
#include <hexa/read_write_lockable.hpp>
#include <hexa/trace.hpp>
#include "random.hpp"

namespace hexa {

class world;

/** Write access to the game world.
 *  This class can only be instanced by hexa::world.  The owner of the
 *  instance has unique write access to a number of chunks.  Once world_write
 *  goes out of scope, the chunks become available for reading again.
 */
class world_write
{
    world& w_;
    std::unique_lock<std::mutex> lock_;
    std::unordered_map<chunk_coordinates, chunk&> cnks_;

    friend class world;

protected:
    world_write (world& w);
    void add (const chunk_coordinates& pos, chunk& cnk);

public:
    world_write(const world_write&) = delete;

#ifdef _MSC_VER
    world_write(world_write&& m)
        : w_(m.w_)
        , lock_(std::move(m.lock_))
        , cnks_(std::move(m.cnks_))
    { }
#else
    world_write(world_write&&) = default;
#endif
    ~world_write();

    chunk& get_chunk(const chunk_coordinates& pos)
    {
        auto found (cnks_.find(pos));
        if (found == cnks_.end())
            throw std::runtime_error("no write access to chunk");

        return found->second;
    }

    bool has_chunk (const chunk_coordinates& pos) const
    {
        return cnks_.count(pos) > 0;
    }

    block& operator[] (const world_coordinates& pos)
    {
        return get_chunk(pos >> cnkshift)[pos % chunk_size];
    }
};

} // namespace hexa
