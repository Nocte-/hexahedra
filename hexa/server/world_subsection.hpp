//---------------------------------------------------------------------------
/// \file   server/world_subsection.hpp
/// \brief  A subsection of the game world.
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

#include <functional>
#include <initializer_list>
#include <unordered_map>

#include <hexa/algorithm.hpp>
#include <hexa/basic_types.hpp>
#include <hexa/chunk.hpp>

namespace hexa {

class world;

class world_subsection_read
{
    friend class world;

protected:
    world_subsection_read() { }

    void add (const world_vector& pos, const chunk& cnk)
    {
        cnks_[pos] = &cnk;
    }

public:
    const chunk&    get_chunk (const world_vector& pos) const
    {
        auto found (cnks_.find(pos));
        if (found == cnks_.end())
            throw std::runtime_error("world_subsection_read::get_chunk: chunk not found");

        return *found->second;
    }

    block           operator[] (const world_vector& pos) const
    {
        return get_chunk(pos >> cnkshift)[pos % chunk_size];
    }

private:
    std::unordered_map<world_vector, const chunk*> cnks_;
};


class world_subsection_write
{
    friend class world;

protected:
    world_subsection_write (const chunk_coordinates& center)
        : center_ (center)
    { }

    void    add (const world_vector& pos, chunk& cnk)
    {
        cnks_[pos] = &cnk;
    }

    void    add_absolute (const chunk_coordinates& pos, chunk& cnk)
    {
        cnks_[pos - center_] = &cnk;
    }

public:
    chunk_coordinates   center() const { return center_; }

    chunk&  center_chunk()
    {
        return get_chunk(center());
    }

    chunk&  get_chunk (const world_vector& pos) const
    {
        auto found (cnks_.find(pos));
        if (found == cnks_.end())
            throw std::runtime_error("world_subsection_write::get_chunk: chunk not found");

        return *found->second;
    }

    chunk&  get_chunk (const chunk_coordinates& pos) const
    {
        return get_chunk(static_cast<world_vector>(pos - center_));
    }

    block&  operator[] (const world_vector& pos) const
    {
        return get_chunk(pos >> cnkshift)[pos % chunk_size];
    }

private:
    chunk_coordinates                        center_;
    std::unordered_map<world_vector, chunk*> cnks_;
};
} // namespace hexa

