//---------------------------------------------------------------------------
/// \file   entity_system_physics.hpp
/// \brief  Systems that take care of the world's physics
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
// Copyright 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include "entity_system.hpp"

namespace hexa {

class storage_i;

/// Walking
void system_walk (es::storage& s, float timestep);

/// Apply forces
void system_force  (es::storage& s, float timestep);

/// Gravity and friction
void system_gravity  (es::storage& s, float timestep);

/// Move entities around
void system_motion (es::storage& s, float timestep);

/// Collision checks against terrain
void system_terrain_collision (es::storage& s,storage_i& terrain);

/// Apply friction from moving over terrain
void system_terrain_friction (es::storage& s, float timestep);

/// Client-side lag compensation
void system_lag_compensate (es::storage& s, float timestep,
                            uint32_t skip = 0xffffffff);

} // namespace hexa

