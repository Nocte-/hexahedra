//---------------------------------------------------------------------------
/// \file physics/euler_integrater.hpp
/// \brief  Simple Euler integration for physics
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

#include "integrater.hpp"

namespace hexa {
namespace physics {

/** A simple Euler integrator for physics simulation. */
class euler_integrater : public integrater
{
public:
    euler_integrater(acceleration_func f) : integrater(f) { }

    state integrate (state s, float delta, float time)
    {
        s.pos += s.v * delta;
        s.v += accel_(s, time) * delta;
        
        return s;
    }
};

}} // namespace hexa::physics

