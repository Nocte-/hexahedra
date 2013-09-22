//---------------------------------------------------------------------------
// entity_system_physics.cpp
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

#include "entity_system_physics.hpp"

#include "algorithm.hpp"
#include "block_types.hpp"
#include "collision.hpp"
#include "geometric.hpp"
#include "voxel_range.hpp"
#include "storage_i.hpp"
#include "trace.hpp"

namespace hexa {

void system_walk (es::storage& s, float timestep)
{
    s.for_each<vector, vector2<float>, yaw_pitch>(entity_system::c_velocity,
                                                  entity_system::c_walk,
                                                  entity_system::c_lookat,
        [&](es::storage::iterator i,
            es::storage::var_ref<vector> v_,
            es::storage::var_ref<vector2<float>> f_,
            es::storage::var_ref<yaw_pitch> l_)
    {
        constexpr float max_walk_speed (5.0f);
        constexpr float max_walk_speed_sq (max_walk_speed * max_walk_speed);

        vector v (v_);
        if (squared_length(v) >= max_walk_speed_sq)
            return;

        vector2<float> f (f_);
        yaw_pitch l (l_);

        vector result (v + rotate(f, -l.x) * timestep);
        float sq_l (squared_length(result));

        if (sq_l <= max_walk_speed_sq)
            v_ = result;
        else
            v_ = (result / std::sqrt(sq_l)) * max_walk_speed;
    });
}

void system_force (es::storage& s, float timestep)
{
    s.for_each<vector, vector>(entity_system::c_velocity,
                               entity_system::c_force,
        [&](es::storage::iterator i,
            es::storage::var_ref<vector> v_,
            es::storage::var_ref<vector> f_)
    {
        vector f (f_);

        // Simple Euler for now.
        v_ += f * timestep;
    });
}

void system_gravity (es::storage& s, float timestep)
{
    s.for_each<vector>(entity_system::c_velocity,
        [&](es::storage::iterator i,
            es::storage::var_ref<vector> v_)
    {
        constexpr float gravity (15.f); // 9.81 doesn't feel right
        constexpr float air_viscosity (0.008f);
        vector v (v_);

        v += vector(0, 0, -gravity) * timestep;
        v -= (v * absolute(v) * air_viscosity) * timestep;
        v_ = v;
    });
}

void system_motion (es::storage& s, float timestep)
{
    s.for_each<wfpos, vector>(entity_system::c_position,
                              entity_system::c_velocity,
        [&](es::storage::iterator i,
            es::storage::var_ref<wfpos> p_,
            es::storage::var_ref<vector> v_)
    {
        vector v (v_);
        wfpos  p (p_);
        p += v * timestep;
        p.normalize();
        p_ = p;
    });
}


static aabb<vector>
make_collision_box (const vector& origin, const vector& size)
{
    return {origin - flat(size), origin + size};
}

void system_terrain_collision (es::storage& s, storage_i& terrain)
{
    s.for_each<wfpos, vector, vector>(entity_system::c_position,
                                      entity_system::c_velocity,
                                      entity_system::c_boundingbox,
        [&](es::storage::iterator i,
            es::storage::var_ref<wfpos>  p_,
            es::storage::var_ref<vector> v_,
            es::storage::var_ref<vector> bb_)
    {
        wfpos p (p_);

        p.normalize();
        auto& offset (p.pos);
        vector v (v_);
        vector bb (bb_);

        aabb<vector> box (make_collision_box(p.frac, bb));

        collision_mesh cm;
        range<world_coordinates> cbr (cast_to<world_coordinates>(box) + offset);
        for (auto b : to_chunk_range(cbr))
        {
            if (is_air_chunk(b, terrain.get_coarse_height(b)))
                continue;

            world_rel_coordinates local_offset ((b * chunk_size) - offset);
            surface_ptr data (terrain.get_surface(b));
            if (data == nullptr)
            {
                // No surface data available yet; block the whole chunk
                cm.emplace_back(aabb<vector>(local_offset, chunk_size), 0x3f);
            }
            else
            {
                for (auto c : data->opaque)
                {
                    vector bp (c.pos + local_offset);

                    const auto& m (material_prop[c.type]);
                    if (!m.bounding_box.empty())
                    {
                        for (auto& part : m.bounding_box)
                            cm.emplace_back(part + bp, 0x3f);
                    }
                    else if (m.is_custom_block())
                    {
                        for (auto& part : m.model)
                            cm.emplace_back((++aabb<vector>(part.box)) / 16.f + bp, 0x3f);
                    }
                    else
                    {
                        cm.emplace_back(bp, 0x3f);
                    }
                }
            }
        }

        vector impact (collide(box, v, cm).impact);

        if (impact != vector::zero())
        {
            // If we bump into something, check if it's a staircase.
            if (impact.z > 0 && (impact.x != 0 || impact.y != 0))
            {
                vector w (v.x, v.y, 0);

                // Move the entity up half a block, and check again.  If
                // the entity can move a greater horizontal distance this
                // way, move it upward.
                //
                vector displace (0, 0, 0.501 + impact.z);
                vector impact2 (collide(box + displace, w, cm).impact);

                if (squared_length(flat(impact2)) < squared_length(flat(impact)))
                {
                    impact.x = impact2.x;
                    impact.y = impact2.y;
                    impact.z = collide(box + impact2, -displace, cm).impact.z;
                }
                else
                {
                  //  break;
                }
            }
        }

        if (impact != vector::zero())
        {
            // Move the entity according to the impact, plus a little
            // nudge to prevent rounding errors.
            p += impact + normalize(impact) * 0.0001f;

            // The impact we get back from the collision detection is the
            // offset in meters.  For the rest of the system, we're going to
            // translate it to the change in velocity, since this is more
            // useful for things like calculating fall damage.
            //
            if (impact.x != 0) { impact.x = -v.x; v.x = 0; }
            if (impact.y != 0) { impact.y = -v.y; v.y = 0; }
            if (impact.z != 0) { impact.z = -v.z; v.z = 0; }
            v_ = v;
        }

        p_ = p;
        s.set(i, entity_system::c_impact, impact);
    });
}

void system_terrain_friction (es::storage& s, float timestep)
{
    s.for_each<vector, vector>(entity_system::c_velocity,
                               entity_system::c_impact,
        [&](es::storage::iterator i,
            es::storage::var_ref<vector> v_,
            es::storage::var_ref<vector> m_)
    {
        constexpr float friction (16.0f);

        vector m (m_);
        vector v (v_);
        if (m.z > 0 && v != vector(0,0,0))
        {
            vector nv (normalize(v) * friction * timestep);

            // If we skip this step, the velocity vector might end up
            // pointing in the opposite direction.  Make sure it comes
            // to a complete standstill.
            if (std::abs(nv.x) > std::abs(v.x)) nv.x = v.x;
            if (std::abs(nv.y) > std::abs(v.y)) nv.y = v.y;
            if (std::abs(nv.z) > std::abs(v.z)) nv.z = v.z;

            v_ = v - nv;
        }
    });
}

void system_lag_compensate (es::storage& s, float timestep, uint32_t skip)
{
    s.for_each<wfpos, vector, last_known_phys>
                             (entity_system::c_position,
                              entity_system::c_velocity,
                              entity_system::c_lag_comp,
        [&](es::storage::iterator i,
            es::storage::var_ref<wfpos> p_,
            es::storage::var_ref<vector> v_,
            es::storage::var_ref<last_known_phys> d_)
    {
        if (i->first == skip)
            return;

        last_known_phys d (d_);

        d.position += d.speed * timestep;
        d_ = d;

        wfpos p (p_);
        vector v (v_);

        auto sq (squared_distance(p, d.position));
        if (sq < 0.00002f || sq > 16.f)
        {
            p_ = d.position;
            v_ = d.speed;
        }
        else
        {
            // Exponentially move towards the destination.  Every
            // halftime seconds, we cover half the remaining distance.
            //
            constexpr float halftime (0.2f);
            float amount (1.0f - std::pow(0.5f, timestep / halftime));
            p_ = lerp(p, d.position, amount);
            v_ = lerp(v, d.speed, amount);
        }
    });
}

} // namespace hexa
