//---------------------------------------------------------------------------
/// \file physics/rk4.hpp
/// \brief  4th order Runge-Kutta integration for physics
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
namespace rk4 {

vector acceleration (const state& s, float t)
{
    return s.pos * -10.f + s.v * -1.f;
}

derivative evaluate (const state& initial, float t, float dt, derivative d)
{
    state s;
    s.pos = initial.pos + d.d_pos * dt;
    s.v   = initial.v   + d.d_v   * dt;

    d.d_pos = s.v;
    d.d_v   = acceleration(s, t + dt);

    return d;
}

state integrate (state s, float t, float dt)
{
    derivative a (evaluate(s, t, dt * 0.0f, derivative());
    derivative b (evaluate(s, t, dt * 0.5f, a);
    derivative c (evaluate(s, t, dt * 0.5f, b);
    derivative d (evaluate(s, t, dt * 1.0f, c);

    vector dpdt (1.f / 6.f * (a.d_pos + 2.f * (b.d_pos + c.d_pos) + d.d_pos);
    vector dvdt (1.f / 6.f * (a.d_v + 2.f * (b.d_v + c.d_v) + d.d_v);

    s.pos += dpdt * dt;
    s.v   += dvdt * dt;

    return s;
}

}}} // namespace hexa::physics::rk4

