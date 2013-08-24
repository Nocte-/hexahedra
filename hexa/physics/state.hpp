//---------------------------------------------------------------------------
/// \file physics/state.hpp
/// \brief The state of a body in a simulation.
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

#include "../basic_types.hpp"

namespace hexa {
namespace physics {

/** The state of a body in a simulation. */
struct state
{
    state(vector speed) : pos (0,0,0), v (speed) { }

    /** Position. */
    vector  pos;
    /** Speed. */
    vector  v;
};

/** The derivative of the state of a body in a simulation. */
struct derivative
{
    /** Delta position (speed). */
    vector d_pos;
    /** Delta speed (acceleration). */
    vector d_v;
};

}} // namespace hexa::physics

