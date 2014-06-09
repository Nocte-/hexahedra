//---------------------------------------------------------------------------
// server/world_write.cpp
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

#include "world_write.hpp"

#include "world.hpp"

namespace hexa
{

world_write::world_write(world& w)
    : w_(w)
    , lock_(w_.single_lock)
{
}

world_write::~world_write()
{
    for (auto& cnk : cnks_) {
        trace(
            "Write commit chunk %1%, fingerprint %2%", cnk.first,
            fnv_hash((const uint8_t*)&*cnk.second.begin(), chunk_volume * 2));
        w_.commit_write(cnk.first);
    }
}

void world_write::add(const chunk_coordinates& pos, chunk& cnk)
{
    cnks_.emplace(pos, cnk);
    trace("Write access to chunk fingerprint %1%",
          fnv_hash((const uint8_t*)&*cnk.begin(), chunk_volume * 2));
}

} // namespace hexa
