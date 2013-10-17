//---------------------------------------------------------------------------
// client/scene.cpp
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

#include "scene.hpp"

#include <iomanip>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/threadpool.hpp>

#include <hexa/algorithm.hpp>
#include <hexa/block_types.hpp>
#include <hexa/voxel_range.hpp>
#include <hexa/surface.hpp>
#include <hexa/trace.hpp>

#include "main_game.hpp"
#include "player.hpp"
#include "types.hpp"


typedef boost::threadpool::prio_task_func  task;

using namespace boost;
using namespace boost::range;

namespace hexa {

extern boost::threadpool::prio_pool pool; // Import from main.cpp

namespace {

static light_data full_bright;

}

template <class type>
direction_type
from_to (const vector3<type>& from, const vector3<type>& to)
{
    assert(manhattan_distance(from, to) == 1);

    const vector3<type> diff (to - from);
    if (diff == vector3<type>( 1, 0, 0))  return dir_east;
    if (diff == vector3<type>(-1, 0, 0))  return dir_west;
    if (diff == vector3<type>( 0, 1, 0))  return dir_north;
    if (diff == vector3<type>( 0,-1, 0))  return dir_south;
    if (diff == vector3<type>( 0, 0, 1))  return dir_up;
    if (diff == vector3<type>( 0, 0,-1))  return dir_down;

    throw std::domain_error("from_to: positions are not next to eachother.");
}

/////////////////////////////////////////////////////////////////////////////

scene::scene(main_game& g)
    : game_             (g)
    , player_pos_       (0, 0, 0)
    , view_dist_        (1)
    , occ_step_         (occluded_.end())
{
    light full;
    full.sunlight = 15;
    full.ambient = 15;
    full.artificial = 15;

    for (int i (0); i < chunk_volume * 6; ++i)
    {
        full_bright.opaque.push_back(full);
        full_bright.transparent.push_back(full);
    }
}

scene::~scene()
{
}

void scene::remove_faraway_terrain()
{
    boost::mutex::scoped_lock lock (terrain_lock_);
    for (auto i (std::begin(terrain_)); i != std::end(terrain_); )
    {
        if (!is_in_view(i->first))
        {
            to_be_deleted.push_back(i->first);
            i = terrain_.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

void scene::view_distance(unsigned int distance)
{
    view_dist_ = distance;
    remove_faraway_terrain();

    // Determine which chunks come into view as the player moves.
    // This is a bit complicated, since we can't know the exact shape of
    // the view range.
    world_vector o (world_chunk_center);
    for (int i (0); i < 6; ++i)
    {
        auto& v(edge_of_view_[i]);
        v.clear();

        for (auto offset : cube_range<world_vector>(distance))
        {
            if (   is_in_view(offset + o,  o)
                && !is_in_view(offset + o + dir_vector[i], o))
            {
                v.push_back(offset);
            }
        }
    }
}

void scene::on_move(chunk_coordinates pos)
{
    if (player_pos_ == pos)
        return;

    auto old_pos (player_pos_);
    player_pos_ = pos;
    remove_faraway_terrain();

    // The area around the current chunk is always visible.
    for (chunk_coordinates c : surroundings(pos, 1))
        make_chunk_visible(c);

    if (manhattan_distance(old_pos, pos) < 5)
    {
        pos = old_pos;

        // Sometimes, the player moves more than one chunk at the time,
        // especially during fps drops.  To make sure everything is still
        // processed in an orderly fashion, we move to the new position
        // step by step.
        //
        while (pos != player_pos_)
        {
            if (pos.x < player_pos_.x)
                ++pos.x;
            else if (pos.x > player_pos_.x)
                --pos.x;
            else if (pos.y < player_pos_.y)
                ++pos.y;
            else if (pos.y > player_pos_.y)
                --pos.y;
            else if (pos.z < player_pos_.z)
                ++pos.z;
            else if (pos.z > player_pos_.z)
                --pos.z;

            // Send out the new occlusion queries for the chunks that just
            // came into the viewing range.
            int direction (from_to(old_pos, pos));

            boost::mutex::scoped_lock lock (terrain_lock_);
            for (world_vector p : edge_of_view_[direction])
            {
                chunk_coordinates abs_pos (p + player_pos_);

                // If this is the topmost part of the terrain, we send out
                // a visibility request.  This helps to make terrain visible
                // earlier when a player walks over a hilltop.
                //
                if (abs_pos.z + 1 == map().get_coarse_height(abs_pos))
                {
                    send_visibility_request(abs_pos);
                    continue;
                }

                // Check the six surrounding chunks
                for (int i (0); i < 6; ++i)
                {
                    world_vector cmp (abs_pos + dir_vector[i]);
                    auto m (terrain_.find(cmp));

                    // If one of the neighbors was visible, check this
                    // one for visibility too.
                    if (   m != terrain_.end()
                        && m->second.status == scene::chunk::visible)
                    {
                        send_visibility_request(abs_pos);
                        break;
                    }
                }
            }

            old_pos = pos;
        }
    }
}

void scene::on_pre_render_loop()
{
}

void scene::on_update_chunk(chunk_coordinates pos)
{
    if (!is_in_view(pos))
    {
        trace("chunk %1% is not in view", world_rel_coordinates(pos - world_chunk_center));
        return;
    }

    if (is_air_chunk(pos, map().get_coarse_height(pos)))
    {
        trace("chunk %1% is air", world_rel_coordinates(pos - world_chunk_center));

        // Tell the renderer this chunk is empty.
        game_.renderer().queue_meshes(pos, game_.make_terrain_mesher(),
                                           game_.make_terrain_mesher());
        return;
    }

    boost::mutex::scoped_lock lock (terrain_lock_);
    if (!map().is_surface_available(pos))
        std::cout << "   On chunk update without a surface" << std::endl;

    auto m (terrain_.find(pos));
    if (m != terrain_.end() && m->second.status == scene::chunk::visible)
    {
        trace("scheduling build_mesh for chunk %1%", world_rel_coordinates(pos - world_chunk_center));
        pool.schedule(task(8, [=]{ build_mesh(pos); }));
    }
}

void scene::on_update_height(map_coordinates pos, chunk_height z,
                             chunk_height old_z)
{
    if (old_z != undefined_height && z > old_z)
        send_visibility_request(chunk_coordinates(pos, old_z));
}

void scene::build_mesh(chunk_coordinates pos)
{
    if (!is_in_view(pos))
    {
        //trace((format("chunk %1% not in view") % world_rel_coordinates(pos - world_chunk_center)).str());
        return;
    }

    auto cnk_height (map().get_coarse_height(pos));
    if (is_air_chunk(pos, cnk_height))
    {
        //trace((format("chunk %1% is air") % world_rel_coordinates(pos - world_chunk_center)).str());
        return;
    }

    light_data*  plm (nullptr);
    lightmap_ptr gcplm;
    surface_ptr  surfaces;

    //assert(map_.is_surface_available(pos));
    surfaces = map().get_surface(pos);
    if (!surfaces)
    {
        //trace((format("chunk %1% has no surface") % world_rel_coordinates(pos - world_chunk_center)).str());
        return;
    }

    gcplm = map().get_lightmap(pos);
    if (gcplm)
        plm = gcplm.get();
    else if (!surfaces->empty())
    {
        //trace((format("chunk %1% has no light map") % world_rel_coordinates(pos - world_chunk_center)).str());
        return;
    }

    {
    boost::mutex::scoped_lock lock (terrain_lock_);
    terrain_[pos].has_meshes = true;
    }

    if (plm == nullptr)
        return;

    auto opaque_mesh (game_.make_terrain_mesher());
    auto transparent_mesh (game_.make_terrain_mesher());

    assert(plm->opaque.size() == count_faces(surfaces->opaque));
    assert(plm->transparent.size() == count_faces(surfaces->transparent));

    {
    auto lmi (plm->opaque.begin());
    auto check (surfaces->opaque.size()); (void)check;
    for(const faces& f : surfaces->opaque)
    {
        assert(surfaces->opaque.size() == check);
        auto pos (f.pos);
        const material& m (material_prop[f.type]);

        if (m.is_custom_block())
        {
            assert(f.dirs == 0x3f);
            std::vector<light> intensities (6);
            for (int d (0); d < 6; ++d)
            {
                if (lmi == plm->opaque.end())
                {
                    assert(false);
                    lmi = plm->opaque.begin();
                }
                intensities[d] = *lmi++;
            }
            opaque_mesh->add_custom_block(pos, m.model, intensities);
        }
        else
        {
            for (int d (0); d < 6; ++d)
            {
                if (!f[d])
                    continue;

                uint16_t tex (m.textures[d]);

                if (lmi == plm->opaque.end())
                {
                    assert(false);
                    lmi = plm->opaque.begin();
                }
                assert(lmi != plm->opaque.end());
                opaque_mesh->add_face(pos, (direction_type)d, tex, *lmi);
                ++lmi;
            }
        }
    }
    assert(lmi == plm->opaque.end());
    }

    {
    auto lmi (plm->transparent.begin());
    for(const faces& f : surfaces->transparent)
    {
        auto pos (f.pos);
        const material& m (material_prop[f.type]);
        for (int d (0); d < 6; ++d)
        {
            if (!f[d])
                continue;

            uint16_t tex (m.textures[d]);
            assert(lmi != plm->transparent.end());
            transparent_mesh->add_face(pos, (direction_type)d, tex, *lmi);
            ++lmi;
        }
    }
    assert(lmi == plm->transparent.end());
    }

    // Because the chunks are built in the background, and we're not hogging
    // the mutex, it is possible the player has moved out of range already
    // by the time we're done.  In that case, don't create the meshes.
    //
    if (is_in_view(pos))
    {
        //trace((format("chunk %1% ready and queued") % world_rel_coordinates(pos - world_chunk_center)).str());
        game_.renderer().queue_meshes(pos, std::move(opaque_mesh),
                                           std::move(transparent_mesh));
    }
}

bool scene::is_in_view(chunk_coordinates pos,
                       chunk_coordinates plr) const
{
    unsigned int m_dist (distance(pos, plr)); // Sphere
    //unsigned int m_dist (manhattan_distance(pos, plr)); // Octahedron
    //unsigned int c_dist (cube_distance(pos, plr)); // Cube

    return m_dist < view_distance();
}

void scene::send_visibility_requests (chunk_coordinates pos)
{
    //trace((format("send visibility requests for the area around %1%") % world_rel_coordinates(pos - world_chunk_center)).str());

    boost::mutex::scoped_lock lock (terrain_lock_);
    for (block_vector d : dir_vector)
    {
        send_visibility_request(pos + d);
    }
}

void scene::send_visibility_request (chunk_coordinates pos)
{
    //trace((format("send visibility request for %1%") % world_rel_coordinates(pos - world_chunk_center)).str());

    if (!is_in_view(pos))
    {
        //trace("  skipped, not in view");
        return;
    }

    auto cnk_height (map().get_coarse_height(pos));
    if (is_air_chunk(pos, cnk_height))
    {
        //trace("  skipped, air chunk");
        return;
    }

    auto m (terrain_.find(pos));
    if (m != terrain_.end())
    {
        if (m->second.status == scene::chunk::pending_query)
        {
            //trace("  skipped, already pending");
            return;
        }

        if (m->second.status == scene::chunk::visible)
        {
            //trace("  skipped, already visible");
            return;
        }

        if (m->second.has_meshes)
        {
            //trace("  skipped, already meshed");
            return;
        }

        m->second.status = scene::chunk::pending_query;
    }
    else
    {
        terrain_[pos].status = scene::chunk::pending_query;
    }

    //trace("queued new OQ for %1%", world_rel_coordinates(pos - world_chunk_center));
    new_occlusion_queries.push_back(pos);
}

void scene::make_chunk_visible (chunk_coordinates pos)
{
    if (!is_in_view(pos))
    {
        //trace("%1% is outside view, schedule for delete", world_rel_coordinates(pos - world_chunk_center));
        to_be_deleted.push_back(pos);
        return;
    }

    if (is_air_chunk(pos, map().get_coarse_height(pos)))
    {
        // Tell the renderer we're not going to put anything here
        //trace("%1% is air, queue empty mesh", world_rel_coordinates(pos - world_chunk_center));
        game_.renderer().queue_meshes(pos, game_.make_terrain_mesher(),
                                           game_.make_terrain_mesher());
        return;
    }

    {
    boost::mutex::scoped_lock lock (terrain_lock_);
    chunk& cnk (terrain_[pos]);
    if (cnk.status == scene::chunk::visible)
    {
        //trace("%1% is already visible, cancel OQ", world_rel_coordinates(pos - world_chunk_center));
        game_.renderer().cancel_occlusion_query(pos);
        return;
    }

    if (cnk.has_meshes)
    {
        //trace("%1% already has a mesh", world_rel_coordinates(pos - world_chunk_center));
        return;
    }

    //trace((format("chunk %1% is now visible") % world_rel_coordinates(pos - world_chunk_center)).str());
    cnk.status = scene::chunk::visible;
    }

    if (map().is_surface_available(pos))
    {
        //trace("   scheduling build mesh");
        pool.schedule(task(8, [=]{ build_mesh(pos); }));
    }
    else
    {
        //trace("  no surface, send request to server");
        game_.request_chunk(pos);
    }
}

void scene::request_chunk (chunk_coordinates pos)
{
    if (!is_in_view(pos))
        return;

    auto cnk_height (map().get_coarse_height(pos));
    if (is_air_chunk(pos, cnk_height))
    {
        // Take a shortcut: since this chunk consists only of air, don't
        // bother asking the server, waiting for the answer, and tesselating
        // it.  Go straight to the visibility requests.
        send_visibility_requests(pos);
        return;
    }

    game_.request_chunk(pos);
}

void scene::consistency_check() const
{
    for (auto i (std::begin(terrain_)); i != std::end(terrain_); ++i)
    {
        if (!is_in_view(i->first))
        {
            std::cout << "ERROR: terrain " << world_vector(i->first - player_pos_) << " " << i->first << std::endl;
        }
    }
    std::cout << "---" << std::endl;
}

storage_i& scene::map()
{
    return game_.world();
}

} // namespace hexa

