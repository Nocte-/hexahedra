//---------------------------------------------------------------------------
/// \file   client/scene.hpp
/// \brief  Manages the 3-D scene around the player.
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

#include <deque>
#include <list>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <boost/thread/mutex.hpp>
#include <boost/optional.hpp>
#include <boost/utility.hpp>

#include <hexa/basic_types.hpp>
#include <hexa/storage_i.hpp>

#include "terrain_mesher_i.hpp"
#include "occlusion_query.hpp"
#include "visibility_test.hpp"

namespace hexa {

class main_game;

struct finished_chunk_mesh
{
    chunk_coordinates                   pos;
    std::unique_ptr<terrain_mesher_i>   opaque_mesh;
    std::unique_ptr<terrain_mesher_i>   transparent_mesh;
};

class scene : boost::noncopyable
{
public:
    mutable boost::mutex                lock;

public:
    scene(main_game& g);

    ~scene();

    void    view_distance(unsigned int distance);

    size_t  view_distance() const { return view_dist_; }

    void    on_pre_render_loop();

    void    on_move(chunk_coordinates pos);

    void    on_update_chunk(chunk_coordinates pos);

    void    on_update_height(map_coordinates pos, chunk_height z);

    void    send_visibility_requests (chunk_coordinates pos);

    chunk_coordinates
            player_pos() const { return player_pos_; }

    void    make_chunk_visible(chunk_coordinates pos);

private:
    bool    is_in_view(chunk_coordinates pos,
                       chunk_coordinates plr) const;

    bool    is_in_view(chunk_coordinates pos) const
                { return is_in_view(pos, player_pos_); }

    void    build_mesh(chunk_coordinates pos);

public:
    std::vector<chunk_coordinates> to_be_deleted;
    std::vector<chunk_coordinates> new_occlusion_queries;

private:
    struct chunk
    {
        enum state_t
        {
            unknown, pending_query, visible, occluded
        };

        chunk()
            : status                (unknown)
            , chunk_request_pending (false)
            , has_meshes            (false)
        { }

        state_t             status;
        bool                chunk_request_pending;
        bool                has_meshes;
    };


    void consistency_check() const;

private:
    typedef std::unordered_map<chunk_coordinates, chunk> terrain_map;

private:
    storage_i& map();

    void remove_faraway_terrain();

    void send_visibility_request (chunk_coordinates pos);

    void request_chunk (chunk_coordinates pos);

    typedef enum
    {
        waiting, busy, finished, inactive
    }
    query_state;

    struct query : public occlusion_query
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

private:
    main_game&          game_;

    chunk_coordinates   player_pos_;
    unsigned int        view_dist_;

    terrain_map         terrain_;
    boost::mutex        terrain_lock_;

    std::array<std::vector<world_vector>, 6> edge_of_view_;

    typedef std::pair<chunk_coordinates, query> query_pair;
    typedef std::list<query_pair> occlist_type;
    occlist_type           occluded_;
    occlist_type::iterator occ_step_;
};

} // namespace hexa

