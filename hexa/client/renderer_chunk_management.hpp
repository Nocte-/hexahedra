//---------------------------------------------------------------------------
/// \file   client/renderer_chunk_management.hpp
/// \brief  Manage a distance-sorted set of chunks for renderers
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
// Copyright 2012-2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <deque>
#include <list>
#include <unordered_map>
#include <vector>

#include <boost/thread/mutex.hpp>
#include <boost/signals2.hpp>

#include <hexa/basic_types.hpp>
#include <hexa/trace.hpp>
#include "occlusion_query.hpp"
#include "renderer_i.hpp"
#include "types.hpp"
#include "vbo.hpp"

namespace hexa {

class renderer_chunk_management : public renderer_i
{
public:
    /// Every chunk coordinate can hold no more than one buffer.
    typedef std::unordered_map<chunk_coordinates, gl::vbo> vbos_t;
    typedef vbos_t::value_type vbos_value_t;

    /// Buffers are kept sorted by their Manhattan distance.
    typedef std::vector<vbos_t> sorted_vbos_t;

    typedef std::unordered_map<chunk_coordinates, occlusion_query> oqs_t;
    typedef oqs_t::value_type oqs_value_t;
    typedef std::vector<oqs_t> sorted_oqs_t;

public:
    renderer_chunk_management()
        : view_dist_(0)
        , plr_pos_(0,0,0)
    { }

    virtual ~renderer_chunk_management() { }

    /// Change the view distance, measured in chunks.
    void view_distance(size_t new_distance)
    {
        boost::mutex::scoped_lock l (lock);
        view_dist_ = new_distance;
        opaque_vbos.resize(view_dist_);
        transparent_vbos.resize(view_dist_);
        occlusion_queries.resize(view_dist_);
    }

    /// Whenever the player moves from one chunk to the next, the chunks
    /// that are outside the view distance must be removed.
    //  Chunks are stored as groups with the same Manhattan distance, so
    //  potentially the shape of the visible world would be a octahedron.
    //  However, the scene object might choose to shape the visible part
    //  of the world differently, hence the need for this function.
    void remove_chunks(const std::vector<chunk_coordinates>& list)
    {
        boost::mutex::scoped_lock l (lock);
        for (auto& pos : list)
        {
            size_t dist (manhattan_distance(plr_pos_, pos));
            if (dist < view_dist_)
            {
                opaque_vbos[dist].erase(pos);
                transparent_vbos[dist].erase(pos);
                occlusion_queries[dist].erase(pos);
            }
        }
    }

    /// Buffers can only be handed to the GPU in the main thread, and
    /// preferably not while we're rendering.  Calling functions can therefore
    /// queue buffers, and they will be picked up automatically at the next
    /// opportunity.
    void queue_meshes(chunk_coordinates pos,
                      terrain_mesher_ptr opaque, terrain_mesher_ptr transparent)
    {
        boost::mutex::scoped_lock lock (vbo_queue_lock);
        vbo_queue.emplace_back(pos, std::move(opaque), std::move(transparent));
    }

    /// Occlusion queries don't involve vertex buffers, so we can add them
    /// directly.
    void add_occlusion_query(chunk_coordinates pos)
    {
        unsigned int dist (manhattan_distance(plr_pos_, pos));
        if (dist >= view_dist_)
        {
            //trace("chunk %1% not in view", world_rel_coordinates(pos - world_chunk_center));
            return;
        }
        boost::mutex::scoped_lock l (lock);
        if (   opaque_vbos[dist].count(pos) != 0
            || transparent_vbos[dist].count(pos) != 0
            || occlusion_queries[dist].count(pos) != 0)
        {
            //trace("chunk %1% already handled in OQ", world_rel_coordinates(pos - world_chunk_center));
            return;
        }
        //trace("new OQ %1%", world_rel_coordinates(pos - world_chunk_center));
        occlusion_queries[dist][pos] = occlusion_query();
    }

    void cancel_occlusion_query(chunk_coordinates pos)
    {
        unsigned int dist (manhattan_distance(plr_pos_, pos));
        if (dist >= view_dist_)
            return;

        boost::mutex::scoped_lock l (lock);
        occlusion_queries[dist].erase(pos);
    }

    void on_update_height(map_coordinates pos, chunk_height z)
    {
        // The coarse height map has been changed.  Cancel all occlusion
        // queries above this limit.
        boost::mutex::scoped_lock l (lock);
        for (auto& d : occlusion_queries)
        {
            for (auto& qry : d)
            {
                if (   //qry.second.state() == occlusion_query::visible
                       map_coordinates(qry.first) == pos
                    && is_air_chunk(qry.first, z))
                {
                    //trace("OQ %1% is above height limit, canceling", world_rel_coordinates(qry.first - world_chunk_center));
                    qry.second.set_state(occlusion_query::air);
                }
            }
        }
    }

    std::deque<chunk_coordinates> get_visible_queries()
    {
        boost::mutex::scoped_lock l (lock);
        std::deque<chunk_coordinates> result;
        for (auto& l : occlusion_queries)
        {
            for (auto& p : l)
            {
                occlusion_query& qry (p.second);

                if (    qry.state() == occlusion_query::cancelled
                    ||  qry.state() == occlusion_query::air)
                {
                    //trace("OQ %1% is canceled/air", world_rel_coordinates(p.first - world_chunk_center));
                    result.push_back(p.first);
                }
                else if (   qry.state() == occlusion_query::busy
                         && qry.is_result_available())
                {
                    if (qry.result() > 8)
                    {
                        //trace("OQ %1% is visible", world_rel_coordinates(p.first - world_chunk_center));
                        qry.set_state(occlusion_query::visible);
                        result.push_back(p.first);
                    }
                    else
                    {
                        qry.set_state(occlusion_query::occluded);
                    }
                }
            }
        }

        return result;
    }

protected:
    /// Go through the queue of waiting chunks, and add them to the
    /// appropriate data structures.
    //  The renderer should call this from within the OpenGL context.
    std::vector<chunk_coordinates> process_vbo_queue()
    {
        std::vector<chunk_coordinates> processed;

        boost::mutex::scoped_lock l (lock);
        boost::mutex::scoped_lock lock(vbo_queue_lock);

        for (auto& b : vbo_queue)
        {
            unsigned int dist (manhattan_distance(plr_pos_, b.pos));
            if (dist < view_dist_)
            {
                occlusion_queries[dist].erase(b.pos);

                if (b.opaque->empty())
                    opaque_vbos[dist].erase(b.pos);
                else
                    opaque_vbos[dist][b.pos] = b.opaque->make_buffer();

                if (b.transparent->empty())
                    transparent_vbos[dist].erase(b.pos);
                else
                    transparent_vbos[dist][b.pos] = b.transparent->make_buffer();

                processed.push_back(b.pos);
            }
        }
        vbo_queue.clear();

        return processed;
    }

    void move_player(chunk_coordinates new_pos)
    {
        if (plr_pos_ == new_pos)
            return;

        boost::mutex::scoped_lock l (lock);
        plr_pos_ = new_pos;

        sort_vbos(opaque_vbos);
        sort_vbos(transparent_vbos);
        sort_vbos(occlusion_queries);
    }

    /// Sort all buffers by their distance to the player.  This function
    /// needs to be called every time the player moves to a different chunk.
    template <class t>
    void sort_vbos (std::vector<t>& set) const
    {
        typedef typename t::value_type elem_t;
        std::vector<t> temp (view_dist_);
        for (t& l : set)
        {
            for (elem_t& p : l)
            {
                size_t dist (manhattan_distance(p.first, plr_pos_));
                if (dist < view_dist_)
                    temp[dist][p.first] = std::move(p.second);
            }
        }
        set.swap(temp);
    }

protected:
    /// Common lock for opaque/transparent/query data structures
    boost::mutex        lock;
    /// All opaque buffers; these are drawn first.
    sorted_vbos_t       opaque_vbos;
    /// All transparent buffers; these are drawn after the opaque ones.
    sorted_vbos_t       transparent_vbos;
    /// The occlusion queries
    sorted_oqs_t        occlusion_queries;

    /// The current view distance, measured in chunks.
    size_t  view_dist_;


    /// A buffer that is queued until the next frame, when it will be
    /// transferred to the GPU.
    struct queued_buffer
    {
        queued_buffer(chunk_coordinates p, terrain_mesher_ptr o,
                      terrain_mesher_ptr t)
            : pos (p)
            , opaque (std::move(o))
            , transparent (std::move(t))
        { }

        chunk_coordinates   pos;
        terrain_mesher_ptr  opaque;
        terrain_mesher_ptr  transparent;
    };

private:

    /// The camera's current chunk position
    chunk_coordinates plr_pos_;

    /// All queued buffers
    std::deque<queued_buffer> vbo_queue;
    /// Terrain meshes are generated in a separate thread (possibly mutiple
    /// threads), so we need to mutex the queue.
    boost::mutex              vbo_queue_lock;
};

} // namespace hexa

