//---------------------------------------------------------------------------
/// \file physics/simulation.hpp
/// \brief  A few functions for determining forces and acceleration.
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

#include "state.hpp"

namespace hexa {
namespace physics {

inline
vector gravity (state, float)
{
    return vector(0, 0, -9.81);
}

inline
vector air_resistance (state, float)
{ 
    return state.v * state.v * -0.1f;
}



}} // namespace hexa::physics

