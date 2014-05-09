//---------------------------------------------------------------------------
/// \file   client/occlusion_manager.hpp
/// \brief  Keep track of occlusion queries
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

#include <unordered_map>
#include <boost/thread/mutex.hpp>
#include <boost/range/algorithm.hpp>
#include "occlusion_query.hpp"

namespace hexa {

class occlusion_manager
{
public:
    typedef enum
    {
        waiting, busy, finished, inactive
    }
    query_state;

    struct query : public gl::occlusion_query
    {
        size_t                      occluded_count;
        size_t                      timer;
        uint8_t                     sides;
        query_state                 state;

        query(uint8_t s = 0)
            : occluded_count(0), timer(0), sides(s), state(waiting)
        { }

        bool was_occluded()
        {
            assert(state == finished);
            bool flag (result() == 0);

            if (flag)
                ++occluded_count;

            return flag;
        }
    };

    typedef std::unordered_map<chunk_coordinates, query> query_map;
    query_map    queries;
    mutable boost::mutex lock;

    void mark_finished_queries()
    {
        using namespace boost::range;

        boost::mutex::scoped_lock query_lock (lock);

        for (auto i (queries.begin()); i != queries.end(); ++i)
        {
            if (i->second.state == busy && i->second.is_result_available())
                i->second.state = finished;
        }
    }

    void remove_finished_queries()
    {
        boost::mutex::scoped_lock query_lock (lock);
        erase_if(queries, [](const query_map::value_type& q)
            { return q.second.state == finished; });
    }

    void info() const
    {
        boost::mutex::scoped_lock query_lock (lock);
        for (auto i (queries.begin()); i != queries.end(); ++i)
        {
            std::cout << i->first << " " << (int)i->second.sides << ": ";
            switch(i->second.state)
            {
                case waiting: std::cout << "waiting"; break;
                case busy: std::cout << "busy"; break;
                case finished: std::cout << "finished"; break;
                case inactive: std::cout << "inactive"; break;
            }
            std::cout << "  " << i->second.occluded_count << " " <<
                        i->second.timer << std::endl;
        }
    }

};

} // namespace hexa

