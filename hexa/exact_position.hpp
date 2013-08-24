//---------------------------------------------------------------------------
/// \file   exact_position.hpp
/// \brief  Sub-voxel precision positioning
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

namespace hexa {

/** A floating point position within the game world.
 *  This class uses both an integer and a floating point component to
 *  keep a reliable precision across the entire world. */
class precise_position
{
public:
    world_coordinates position() const
        { return position_; }

    vector position_fraction() const
        { return position_fraction_; }

    vector3<double> world_position() const
        { return position() + position_fraction(); }

    vector rel_world_position(const world_coordinates& o) const
        { return world_vector(position() - o) + position_fraction(); }

    chunk_coordinates chunk_position() const
        { return position_ / chunk_size; }

    vector position_relative_in_chunk() const
        { return position_fraction() + vector(position() % chunk_size); }

private:
    /** The voxel itself. */
    world_coordinates   position_;
    /** The position within the voxel (ranged 0..1). */
    vector              fraction_;
};

} // namespace hexa

