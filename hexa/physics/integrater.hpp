//---------------------------------------------------------------------------
/// \file physics/integrater.hpp
/// \brief Base class for integraters
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

#include <cmath>
#include "state.hpp"

namespace hexa {
namespace physics {

/** Base class for integraters.
 *  An integrater's job is to determine the new position and velocity for
 *  all bodies in the simulation after a given timestep and acceleration. */
class integrater
{
public:
    typedef std::function<vector(state, float)> acceleration_func;

    integrater (acceleration_func accel)
        : accel_(accel)
    {}

    virtual state integrate (state s, float delta, float time) = 0;

    state multistep (state s, float delta, float max_delta, float time)
    {
        int steps (1);
        if (max_delta < delta)
            steps = std::ceil(delta / max_delta);

        float step_delta (delta / steps);
        for (int i (0); i < steps; ++i)
            s = integrate(s, step_delta, time + i * step_delta);

        return s;
    }

protected:
    acceleration_func accel_;
};

}} // namespace hexa::physics

