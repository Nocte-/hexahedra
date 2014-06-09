//---------------------------------------------------------------------------
/// \file   player_base.hpp
/// \brief  Base class for player objects
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

#include <vector>
#include "basic_types.hpp"
#include "hotbar_slot.hpp"
#include "vector3.hpp"

namespace hexa
{

/** */
class player_base
{
public:
    vector3<double> world_position() const
    {
        return position + position_fraction;
    }

    vector3<float> rel_world_position(const world_vector& o) const
    {
        return vector3<int32_t>(position - o) + position_fraction;
    }

    chunk_coordinates chunk_position() const { return position >> cnkshift; }

    vector3<float> position_relative_in_chunk() const
    {
        return (position % chunk_size) + position_fraction;
    }

public:
    world_coordinates position;
    vector3<float> position_fraction;
    yaw_pitch head_angle;
    std::vector<hotbar_slot> hotbar;
};

} // namespace hexa
