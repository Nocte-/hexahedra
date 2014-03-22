//---------------------------------------------------------------------------
/// \file   server/extract_surface.hpp
/// \brief  Find all potential visible faces in a chunk and turn them into
///         a surface.
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

#include "world_subsection.hpp"
#include <hexa/surface.hpp>

namespace hexa {

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
extract_opaque_surface (const world_subsection_read& terrain);

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
extract_transparent_surface (const world_subsection_read& terrain);

} // namespace hexa

