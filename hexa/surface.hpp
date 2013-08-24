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
// Copyright 2012, nocte@hippie.nu
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
    surface opaque;
    surface transparent;

public:
    surface_data () { }
    surface_data (surface o, surface t) : opaque(o), transparent(t) { }

    bool empty() const
        { return opaque.empty() && transparent.empty(); }

    template <class archive>
    archive& serialize(archive& ar)
        { return ar(opaque)(transparent); }
};

typedef std::shared_ptr<surface_data> surface_ptr;


template <class t> class neighborhood;

/** Find all potentially visible faces in a chunk.
 *  If two solid blocks are right next to each other, the two touching
 *  faces will never be visible.  This function will only return faces
 *  that can be seen, the ones that are right next to a transparent block
 *  type.  Because the visibility of the faces at the outer edges of the
 *  chunk can only be determined by looking at the blocks in the chunk
 *  right next to it, this function requires a neighborhood<> as its
 *  input.
 * @param terrain  The chunk to determine the surface of, with its
 *                 six immediate neighboring chunks
 * @return The potentially visible surface */
surface
extract_surface (const neighborhood<chunk_ptr>& terrain);

/** Find all potentially visible opaque faces in a chunk.
 *  If two solid blocks are right next to each other, the two touching
 *  faces will never be visible.  This function will only return faces
 *  that can be seen, the ones that are right next to a transparent block
 *  type.  Because the visibility of the faces at the outer edges of the
 *  chunk can only be determined by looking at the blocks in the chunk
 *  right next to it, this function requires a neighborhood<> as its
 *  input.
 * @param terrain  The chunk to determine the surface of, with its
 *                 six immediate neighboring chunks
 * @return The potentially visible surface */
surface
extract_opaque_surface (const neighborhood<chunk_ptr>& terrain);

/** Find all potentially visible transparent faces in a chunk.
 *  If two solid blocks are right next to each other, the two touching
 *  faces will never be visible.  This function will only return faces
 *  that can be seen, the ones that are right next to a transparent block
 *  type.  Because the visibility of the faces at the outer edges of the
 *  chunk can only be determined by looking at the blocks in the chunk
 *  right next to it, this function requires a neighborhood<> as its
 *  input.
 * @param terrain  The chunk to determine the surface of, with its
 *                 six immediate neighboring chunks
 * @return The potentially visible surface */
surface
extract_transparent_surface (const neighborhood<chunk_ptr>& terrain);

/** Count the number of faces in a surface. */
size_t count_faces (const surface& s);


} // namespace hexa

