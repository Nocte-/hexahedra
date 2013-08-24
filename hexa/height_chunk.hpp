//---------------------------------------------------------------------------
/// \file   height_chunk.hpp
/// \brief  A 16x16 part of the world's heightmap.
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
#include <cassert>
#include <utility>

#include "basic_types.hpp"

namespace hexa {

/** A 16x16 part of the world's height map. */
class height_chunk : public std::array<uint32_t, 16*16/*chunk_area*/>
{
    typedef std::array<uint32_t, 16*16/*chunk_area*/>  array_t;

public:
    typedef array_t::value_type        value_type;
    typedef array_t::iterator          iterator;
    typedef array_t::const_iterator    const_iterator;

    using array_t::operator[];

public:
    inline
    value_type&         operator[](const map_coordinates& pos)
        { return operator()(pos.x, pos.y); }

    inline
    value_type          operator[](const map_coordinates& pos) const
        { return operator()(pos.x, pos.y); }

    inline
    value_type&         operator()(uint8_t x, uint8_t y)
    {
        assert (x < chunk_size);
        assert (y < chunk_size);

        return array_t::operator[](y + x * chunk_size);
    }

    inline
    value_type          operator()(uint8_t x, uint8_t y) const
    {
        assert (x < chunk_size);
        assert (y < chunk_size);

        return array_t::operator[](y + x * chunk_size);
    }
};

} // namespace hexa

