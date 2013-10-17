//---------------------------------------------------------------------------
// hexa/locked_subsection.cpp
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

#include "locked_subsection.hpp"

namespace hexa {

locked_subsection::locked_subsection(std::vector<boost::unique_lock<boost::mutex>>&& locks,
                                     std::unordered_map<chunk_coordinates, chunk_ptr>&& chunks)
    : base(std::move(chunks))
    , locked_(std::move(locks))
{
}

locked_subsection::~locked_subsection()
{
    for (auto& l : locked_)
        l.unlock();
}

void locked_subsection::add (chunk_coordinates pos, chunk_ptr p)
{
    set_chunk(pos, p);
    locked_.emplace_back(p->lock());
}

}
