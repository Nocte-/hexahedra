//---------------------------------------------------------------------------
// hexa/client/scene.cpp
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

#include "scene.hpp"

#include <set>

#include <hexa/algorithm.hpp>
#include <hexa/block_types.hpp>
#include <hexa/lightmap.hpp>
#include <hexa/voxel_range.hpp>
#include <hexa/surface.hpp>
#include <hexa/trace.hpp>

#include "chunk_cache.hpp"
#include "main_game.hpp"
#include "types.hpp"


namespace hexa {

scene::scene (main_game& g)
    : game_     (g)
    , threads_  (4)
{
    terrain_.on_before_update.connect([&](chunk_coordinates, chunk_data& d)
    {
        awaiting_cleanup_.emplace_back(std::move(d));
    });

    terrain_.on_remove.connect([&](chunk_coordinates, chunk_data& d)
    {
        awaiting_cleanup_.emplace_back(std::move(d));
    });
}

scene::~scene()
{
}

void
scene::view_distance (size_t distance)
{
    auto old_distance (terrain_.view_radius());
    terrain_.view_radius(distance);

    if (old_distance < distance)
    {
        ///\todo
    }

    // Determine which chunks come into view as the camera moves.
    auto o (terrain_.center());
    for (int i (0); i < 6; ++i)
    {
        auto& v(edge_of_view_[i]);
        v.clear();

        for (auto offset : cube_range<world_vector>(distance))
        {
            if (    terrain_.is_inside(offset + o)
                && !terrain_.is_inside(offset + o + dir_vector[i]))
            {
                v.push_back(offset);
            }
        }
    }
}

void
scene::move_camera_to (chunk_coordinates pos)
{
    std::unique_lock<std::mutex> locked (lock);

    auto old_pos (terrain_.center());
    if (old_pos == pos)
        return;

    terrain_.center(pos);

    // The area around the camera's chunk is always visible.
    for (chunk_coordinates c : surroundings(pos, 1))
        chunk_became_visible(c);

    if (manhattan_distance(old_pos, pos) < 5)
    {
        // Sometimes, the camera moves more than one chunk at the time,
        // especially during fps drops.  To make sure everything is still
        // processed in an orderly fashion, we move to the new position
        // step by step.
        //
        auto step (old_pos);
        while (step != pos)
        {
            int direction (-1);
            if (step.x < pos.x)        { ++step.x; direction = 0; }
            else if (step.x > pos.x)   { --step.x; direction = 1; }
            else if (step.y < pos.y)   { ++step.y; direction = 2; }
            else if (step.y > pos.y)   { --step.y; direction = 3; }
            else if (step.z < pos.z)   { ++step.z; direction = 4; }
            else if (step.z > pos.z)   { --step.z; direction = 5; }
            assert(direction != -1);

            for (world_vector p : edge_of_view_[direction])
            {
                chunk_coordinates abs_pos (step + p);

                // Check the six surrounding chunks
                for (int i (0); i < 6; ++i)
                {
                    chunk_coordinates cmp (abs_pos + dir_vector[i]);
                    if (terrain_.has(cmp))
                    {
                        auto& info (terrain_.get(cmp));

                        // If one of the neighbors was visible, check this
                        // one for visibility too.
                        if (info.opaque || info.transparent)
                            make_occlusion_query(cmp);
                    }
                }
            }
        }
    }
}

void
scene::set (chunk_coordinates pos, const surface_data& surface,
            const light_data& light)
{
    if (!terrain_.is_inside(pos))
        return;

    std::unique_lock<std::mutex> locked (pending_lock_);
    pending_.emplace_back(threads_.enqueue([=]{ return build_mesh(pos, surface, light); }));
}

void
scene::set_coarse_height (map_coordinates pos,
                          chunk_height h, chunk_height old_height)
{
    std::unique_lock<std::mutex> locked (lock);

    if (old_height == undefined_height)
    {
        // First time the height map is set; query the topmost chunk.
        make_occlusion_query(chunk_coordinates(pos.x, pos.y, h - 1));
    }
    else if (h > old_height)
    {
        // The terrain got higher, query the first and last chunk of the
        // new part of the column.
        make_occlusion_query(chunk_coordinates(pos.x, pos.y, old_height));
        make_occlusion_query(chunk_coordinates(pos.x, pos.y, h - 1));
    }
    else if (h < old_height)
    {
        // Terrain is now lower than before, remove chunks.
        for (auto i (old_height - 1); i >= h; --i)
            terrain_.remove(chunk_coordinates(pos.x, pos.y, i));
    }
}

void
scene::pre_render()
{
    std::unique_lock<std::mutex> locked1 (pending_lock_);
    std::unique_lock<std::mutex> locked2 (lock);

    awaiting_cleanup_.clear();

    for (auto i (std::begin(pending_)); i != std::end(pending_); )
    {
        if (is_ready(*i))
        {
            place_finished_mesh(i->get());
            i = pending_.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

void
scene::place_finished_mesh (const finished_mesh& mesh)
{
    // Generate the VBOs, and throw out the old occlusion query object.
    if (!terrain_.set(mesh.pos,
                      { mesh.opaque->make_buffer(),
                        mesh.transparent->make_buffer(),
                        gl::occlusion_query(false) }))
    {
        trace("WARNING: set() returned false");
    }
    else
    {
        // Set up occlusion queries for the surrounding chunks
        for (auto& p : dir_vector)
        {
            chunk_coordinates pos (mesh.pos + p);

            // If this happens to be an air chunk, we make a request for the
            // topmost chunk of that column.
            auto ch (game_.map().get_coarse_height(pos));
            if (ch != undefined_height)
                pos.z = std::min(pos.z, ch - 1);

            if (terrain_.is_inside(pos) && !terrain_.has(pos))
                make_occlusion_query(pos);
        }
    }
}

void
scene::post_render()
{
    std::unique_lock<std::mutex> locked (lock);
    terrain_.for_each([&](distance_sorted_map<chunk_data>::value_type& p)
    {
        auto& qry (p.second.occ_qry);
        if (qry.is_result_available())
        {
            if (qry.result() > 4)
            {
                qry.set_state(gl::occlusion_query::visible);
                chunk_became_visible(p.first);
            }
            else
            {
                qry.set_state(gl::occlusion_query::idle);
            }
        }
    });
}

void
scene::request_chunk_from_server (chunk_coordinates pos) const
{
    game_.request_chunk(pos);
}


scene::finished_mesh
scene::build_mesh (chunk_coordinates pos, const surface_data& surfaces,
                   const light_data& lm) const
{
    // Request two mesher objects from the renderer.
    auto opaque_mesh      (game_.make_terrain_mesher());
    auto transparent_mesh (game_.make_terrain_mesher());

    // Sanity check, light map should be the same size as the surface.
    assert(lm.opaque.size() == count_faces(surfaces.opaque));
    assert(lm.transparent.size() == count_faces(surfaces.transparent));

    // Pass the opaque surfaces and light intensities to the mesher.
    {
    auto lmi (lm.opaque.begin());
    auto check (surfaces.opaque.size()); (void)check;
    for(const faces& f : surfaces.opaque)
    {
        assert(surfaces.opaque.size() == check);
        auto pos (f.pos);
        const material& m (material_prop[f.type]);

        if (m.is_custom_block())
        {
            assert(f.dirs == 0x3f);
            std::vector<light> intensities (6);
            for (int d (0); d < 6; ++d)
            {
                if (lmi == lm.opaque.end())
                {
                    // Light map data is inconsistent with surface data
                    assert(false);
                    lmi = lm.opaque.begin();
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

                if (lmi == lm.opaque.end())
                {
                    // Light map data is inconsistent with surface data
                    assert(false);
                    lmi = lm.opaque.begin();
                }
                assert(lmi != lm.opaque.end());
                opaque_mesh->add_face(pos, (direction_type)d, tex, *lmi);
                ++lmi;
            }
        }
    }
    assert(lmi == lm.opaque.end());
    }

    // Same for the transparent parts.
    {
    auto lmi (lm.transparent.begin());
    for(const faces& f : surfaces.transparent)
    {
        auto pos (f.pos);
        const material& m (material_prop[f.type]);
        for (int d (0); d < 6; ++d)
        {
            if (!f[d])
                continue;

            uint16_t tex (m.textures[d]);
            assert(lmi != lm.transparent.end());
            transparent_mesh->add_face(pos, (direction_type)d, tex, *lmi);
            ++lmi;
        }
    }
    assert(lmi == lm.transparent.end());
    }

    // Return the results as a std::future, so the render loop can pick
    // it up at the next round.
    return { pos, std::move(opaque_mesh), std::move(transparent_mesh) };
}

void
scene::chunk_became_visible (chunk_coordinates pos)
{
    if (!terrain_.is_inside(pos))
        return;

    if (terrain_.has(pos))
    {
        // Make sure the chunk wasn't already visible.
        auto& info (terrain_.get(pos));
        if (info.opaque || info.transparent)
        {
            return;
        }
    }

    // If we have the surface in storage, use it.
    auto& m (game_.map());
    if (   m.is_surface_available(pos)
        && m.is_lightmap_available(pos))
    {
        set(pos, m.get_surface(pos), m.get_lightmap(pos));
    }

    // Ask the server for the data, even if we already had it, just in case
    // there's a newer version available.
    request_chunk_from_server(pos);
}

void
scene::make_occlusion_query (chunk_coordinates pos)
{
    terrain_.set(pos, {gl::vbo(), gl::vbo(), gl::occlusion_query(true)});
}

} // namespace hexa
