//---------------------------------------------------------------------------
// hexa/server/clock.cpp
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

#include "clock.hpp"

#include <boost/chrono.hpp>

using namespace boost::chrono;

namespace hexa
{
namespace clock
{

namespace
{

steady_clock::time_point start_;

} // anonymous namespace

uint64_t epoch()
{
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
        .count();
}

void init()
{
    start_ = steady_clock::now();
}

uint64_t now()
{
    return duration_cast<milliseconds>(steady_clock::now() - start_).count();
}

uint64_t game_time()
{
    return static_cast<uint64_t>(now() / 10);
}

gameclock_t client_time(uint64_t client_offset)
{
    return static_cast<gameclock_t>(now() - client_offset);
}

} // namespace clock
} // namespace hexa
