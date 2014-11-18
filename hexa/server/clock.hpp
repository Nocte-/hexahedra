//---------------------------------------------------------------------------
/// \file   server/clock.hpp
/// \brief  This clock is kept synchronized with the server's clock
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

#include "../basic_types.hpp"

namespace hexa
{
namespace clock
{

/** Milliseconds since epoch. */
uint64_t epoch();

/** Set the current time as the reference time. */
void init();

/** Milliseconds since clock::init() was called. */
uint64_t now();

/** Time in game ticks (1/100th of a second). */
uint64_t game_time();

/** Reference time for a client. */
gameclock_t client_time(uint64_t client_offset);

} // namespace clock
} // namespace hexa
