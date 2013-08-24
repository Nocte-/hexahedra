//---------------------------------------------------------------------------
/// \file   server/work_queue.hpp
/// \brief  Work queue
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

#include <vector>
#include <hexa/basic_types.hpp>

namespace hexa {

class job : boost::noncopyable
{
public:
    typedef uint16_t   priority_t;

    enum
    {
        nop = 0,
        finished = 1,
        generate_terrain,
        extract_surface,
        generate_lightmap,
        shutdown,
        last
    }
    action;
    
    priority_t          priority;
    chunk_coordinates   pos;
    int                 param;

public:
    bool operator< (const job& compare) const
        { return priority < compare.priority; }
};

/** The job queue keeps track of what needs to be done, and hands it
 ** over to a thread pool.
 *  It does this somewhat intelligently.  Whenever a new job is scheduled,
 *  it first checks whether it had been scheduled before.  If so, the one
 *  with the lowest priority is cancelled.  Also, it makes sure no two
 *  jobs are working on the same data.  If one job is building a surface,
 *  and another job is at the front of the queue to build a light map of
 *  that same chunk, it leaves that job sitting in the queue and grabs the
 *  second. */
class job_queue : boost::noncopyable
{
public:
    typedef std::function<void(const job&)> func;

public:
    job_queue(unsigned int threads);

    void schedule (job&& new_job);

private:
    std::vector<job>  queue_;
    std::vector<job>  active_;
    std::vector<func> handlers_;
};

} // namespace hexa

