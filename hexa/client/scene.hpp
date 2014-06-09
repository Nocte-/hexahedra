//---------------------------------------------------------------------------
/// \file   hexa/client/scene.hpp
/// \brief  Manages the scene around the camera.
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

#include <list>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <hexa/basic_types.hpp>
#include <hexa/distance_sorted_map.hpp>
#include <hexa/threadpool.hpp>

#include "terrain_mesher_i.hpp"
#include "occlusion_query.hpp"

namespace hexa
{

class main_game;
class surface_data;
class light_data;

class scene
{
private:
    // Every entry in \a dsmap has a VBO for the opaque and transparent
    // chunk mesh, and an occlusion query.  Note that the VBOs and the OQ
    // start out undefined.
    struct chunk_data
    {
        gl::vbo opaque;
        gl::vbo transparent;
        gl::occlusion_query occ_qry;

#if defined(_MSC_VER)
        chunk_data() {}
        chunk_data(gl::vbo&& o, gl::vbo&& t, gl::occlusion_query& q)
            : opaque(std::move(o))
            , transparent(std::move(t))
            , occ_qry(std::move(q))
        {
        }
        chunk_data(chunk_data&& m)
            : opaque(std::move(m.opaque))
            , transparent(std::move(m.transparent))
            , occ_qry(std::move(occ_qry))
        {
        }
        chunk_data& operator=(chunk_data&& m)
        {
            if (&m != this) {
                opaque = std::move(m.opaque);
                transparent = std::move(m.transparent);
                occ_qry = std::move(occ_qry);
            }
            return *this;
        }
#endif
    };

    typedef distance_sorted_map<chunk_data> dsmap;

public:
    mutable std::mutex lock;

public:
    scene(main_game& g);
    ~scene();

    void view_distance(size_t distance);

    size_t view_distance() const { return terrain_.view_radius(); }

    void set_chunk_offset(chunk_coordinates new_offset)
    {
        chunk_offset_ = new_offset;
    }

    void move_camera_to(chunk_coordinates pos);

    void set(chunk_coordinates pos, const surface_data& surface,
             const light_data& light);

    void set_coarse_height(map_coordinates pos, chunk_height h,
                           chunk_height old_height);

    void pre_render();

    void post_render();

public:
    template <typename func>
    void for_each_opaque_vbo(func op) const
    {
        terrain_.for_each([&](const dsmap::value_type& info) {
            if (info.second.opaque)
                op(info.first, info.second.opaque);
        });
    }

    template <typename func>
    void for_each_transparent_vbo(func op) const
    {
        terrain_.for_each_reverse([&](const dsmap::value_type& info) {
            if (info.second.transparent)
                op(info.first, info.second.transparent);
        });
    }

    template <typename func>
    void for_each_occlusion_query(func op)
    {
        terrain_.for_each([&](dsmap::value_type& info) {
            if (info.second.occ_qry)
                op(info.first, info.second.occ_qry);
        });
    }

private:
    void chunk_became_visible(chunk_coordinates pos);

    void make_occlusion_query(chunk_coordinates pos);

private:
    struct finished_mesh
    {
        chunk_coordinates pos;
        std::unique_ptr<terrain_mesher_i> opaque;
        std::unique_ptr<terrain_mesher_i> transparent;

#if defined(_MSC_VER)
        finished_mesh() {}

        finished_mesh(chunk_coordinates p,
                      std::unique_ptr<terrain_mesher_i>&& o,
                      std::unique_ptr<terrain_mesher_i>&& t)
            : pos(p)
            , opaque(std::move(o))
            , transparent(std::move(t))
        {
        }

        finished_mesh(finished_mesh&& m)
            : pos(m.pos)
            , opaque(std::move(m.opaque))
            , transparent(std::move(m.transparent))
        {
        }

        finished_mesh& operator=(finished_mesh&& m)
        {
            if (&m != this) {
                pos = m.pos;
                opaque = std::move(m.opaque);
                transparent = std::move(m.transparent);
            }
            return *this;
        }
#endif
    };

    finished_mesh build_mesh(chunk_coordinates pos,
                             const surface_data& surface,
                             const light_data& light) const;

    void place_finished_mesh(const finished_mesh& m);

    void request_chunk_from_server(chunk_coordinates pos) const;

private:
    main_game& game_;
    dsmap terrain_;
    threadpool threads_;
    chunk_coordinates chunk_offset_;

    std::array<std::vector<world_vector>, 6> edge_of_view_;

    std::mutex pending_lock_;
    std::list<std::future<finished_mesh>> pending_;
    std::list<chunk_data> awaiting_cleanup_;
};

} // namespace hexa
