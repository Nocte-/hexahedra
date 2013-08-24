//---------------------------------------------------------------------------
// hexa/client/clock.cpp
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

#include "clock.hpp"

#include <chrono>

using namespace std::chrono;

namespace hexa {
namespace clock {

namespace {

steady_clock::time_point start_;
int32_t adjustment_;

uint32_t elapsed_()
{
    return duration_cast<milliseconds>(steady_clock::now() - start_).count();
}

} // anonymous namespace

void init()
{
    start_ = steady_clock::now();
    adjustment_ = 0;
}

void sync(clientclock_t time)
{
    adjustment_ = time - elapsed_();
}

clientclock_t time()
{
    return elapsed_() + adjustment_;
}

}} // namespace hexa::clock

