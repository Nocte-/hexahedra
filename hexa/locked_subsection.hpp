//---------------------------------------------------------------------------
/// \file   hexa/locked_subsection.hpp
/// \brief  A subsection of the world that is locked by mutexes.
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
// Copyright (C) 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <unordered_map>
#include <vector>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include "chunk.hpp"
#include "world_subsection.hpp"

namespace hexa {

class world;

/** This lock can be acquired through world::lock_region(). */
class locked_subsection : public world_subsection<chunk_ptr>
{
    friend class world;
    typedef world_subsection<chunk_ptr> base;

public:
    locked_subsection(const locked_subsection&) = delete;

    locked_subsection(locked_subsection&& m)
        : base(std::move(m))
        , locked_(std::move(m.locked_))
    { }

    ~locked_subsection();

protected:
    locked_subsection() { }

    locked_subsection(std::vector<boost::unique_lock<boost::mutex>>&& locks,
                      std::unordered_map<chunk_coordinates, chunk_ptr>&& chunks);

    void add (chunk_coordinates pos, chunk_ptr p);


private:
    std::vector<boost::unique_lock<boost::mutex>> locked_;
};

} // namespace hexa
