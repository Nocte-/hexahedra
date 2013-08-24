//---------------------------------------------------------------------------
/// \file   pos_dir.hpp
/// \brief  Combination of a position and a direction.
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
// Copyright (C) 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <array>
#include <iostream>
#include "basic_types.hpp"

namespace hexa {

template <class pos_t>
class pos_dir
{
public:
    typedef pos_t   pos_type;

    pos_dir (const pos_t& p, direction_type d) : pos(p), dir(d) {}

    bool operator== (const pos_dir<pos_t>& compare) const
        { return pos == compare.pos && dir == compare.dir; }

    pos_t target() const
        { return pos + dir_vector[dir]; }

    pos_dir opposite() const 
        { return pos_dir(target(), static_cast<direction_type>(dir ^ 1)); }

public:
    pos_t           pos;
    direction_type  dir;
};

template <class pos_t>
class pos_dirs
{
public:
    typedef pos_t   pos_type;

    pos_dirs () : pos(0,0,0), dirs(0) {}

    pos_dirs (const pos_t& p, uint8_t d) 
        : pos(p), dirs(d) 
    {
        assert(valid());
    }

    bool operator== (const pos_dirs<pos_t>& compare) const
        { return pos == compare.pos && dirs == compare.dirs; }

    bool operator== (const pos_t& compare) const
        { return pos == compare; }

    bool operator[] (direction_type dir) const
        { return operator[](static_cast<uint8_t>(dir)); }

    bool operator[] (uint8_t dir) const
    { 
        assert(valid());
        return dirs & (1 << dir); 
    }

    std::vector<direction_type> directions() const
    {
        std::vector<direction_type> result;
        for (int d (0); d < 6; ++d)
        {
            uint8_t dir (1 << d);
            if (dirs & dir) 
                result.push_back(static_cast<direction_type>(dir));
        }
        return result;
    }

    std::vector<pos_t> targets() const
    {
        std::vector<pos_t> result;
        for (int d (0); d < 6; ++d)
        {
            if (dirs & (1 << d)) 
                result.push_back(pos + dir_vector[d]);
        }
        return result;
    }

    bool valid() const
    {
        assert(dirs < 64);
        assert(pos.x < chunk_size);
        assert(pos.y < chunk_size);
        assert(pos.z < chunk_size);
        return true;
    }

public:
    pos_t    pos;
    uint8_t  dirs;
};

} // namespace hexa

namespace std
{

inline
ostream& operator<< (ostream& str, hexa::direction_type d)
{
    static const char letter[] = { 'E', 'W', 'N', 'S', 'U', 'D' };
    return str << letter[(int)d];
}

template <class t>
ostream& operator<< (ostream& str, const hexa::pos_dir<t>& pd)
{
    return str << pd.pos << '/' << pd.dir;
}

}

