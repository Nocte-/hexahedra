//---------------------------------------------------------------------------
/// \file   surface.hpp
/// \brief  A surface is a set of visible chunk faces
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

#include <memory>
#include <vector>
#include "basic_types.hpp"
#include "chunk.hpp"
#include "pos_dir.hpp"

namespace hexa {

/** All exposed faces of a block.
 *  The direction mask consists of six bits, one for every direction, and
 *  therefor has a range of 0..63.  The magic value 255 is used to mark a
 *  block with a custom 3-D model (exposed faces and collision detection
 *  work different for those). */
class faces : public pos_dirs<chunk_index>
{
public:
    uint16_t                type;

    faces ()
        : pos_dirs(chunk_index(0,0,0), 0), type (0)
    { }

    faces (pos_dirs<chunk_index> f, uint16_t t)
        : pos_dirs(f), type (t)
    { }

    faces (chunk_index i, uint8_t d, uint16_t t)
        : pos_dirs(i, d), type (t)
    { }

    template <class archive>
    archive& serialize(archive& ar)
        { return ar(type)(pos)(dirs); }
};

/** A list of faces in a chunk. */
typedef std::vector<faces> surface;

/** Bundle the surfaces of the opaque and transparent parts in one object. */
class surface_data
{
public:
    surface     opaque;
    surface     transparent;

public:
    surface_data () { }
    surface_data (surface&& o, surface&& t)
        : opaque(std::move(o))
        , transparent(std::move(t))
    { }

    surface_data(surface_data&& m)
        : opaque(std::move(m.opaque))
        , transparent(std::move(m.transparent))
    { }

    surface_data(const surface_data& ) = default;

    surface_data& operator= (surface_data&& m)
    {
        if (this != &m)
        {
            opaque = std::move(m.opaque);
            transparent = std::move(m.transparent);
        }
        return *this;
    }

    surface_data& operator= (const surface_data& m) = default;

    bool operator== (const surface_data& c) const
    {
        return opaque == c.opaque && transparent == c.transparent;
    }

    bool empty() const
        { return opaque.empty() && transparent.empty(); }

    template <class archive>
    archive& serialize(archive& ar)
        { return ar(opaque)(transparent); }
};

/** Count the number of faces in a surface. */
size_t count_faces (const surface& s);


} // namespace hexa

