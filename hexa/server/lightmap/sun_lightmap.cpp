//---------------------------------------------------------------------------
// server/light/sun_lightmap.cpp
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
// Copyright 2012-2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "sun_lightmap.hpp"

#include <algorithm>
#include <array>

#include <hexa/voxel_algorithm.hpp>
#include <hexa/voxel_range.hpp>
#include <hexa/trace.hpp>

#include "../world.hpp"
#include "../world_lightmap_access.hpp"

using namespace boost::property_tree;

namespace hexa {

typedef std::array<unsigned int, 3> triangle;

namespace {

float opacity (uint16_t t)
{
    return 1.0f - (material_prop[t].transparency / 255.f);
}

} // anonymous namespace


////////////////////////////////////////////////////////////////////////////


sun_lightmap::sun_lightmap (world& c, const ptree& conf)
    : lightmap_generator_i (c, conf)
    , direction_ (-0.4f, 0.75f)
    , radius_    (3.0f * 0.01745f)
{
    detail_levels_.emplace_back(generate(10, 0));
    detail_levels_.emplace_back(generate(60, 1));
    detail_levels_.emplace_back(generate(200, 2));
}

sun_lightmap::~sun_lightmap ()
{ }

void
sun_lightmap::add (sun_lightmap::rays& r, float raylen, yaw_pitch dir2) const
{
    const vector half (0.5, 0.5, 0.5);
    for (int d (0); d < 6; ++d)
    {
        vector normal (dir_vector[d]);
        vector origin (half + normal * 0.6f);
        vector dir    (from_spherical(dir2.x, dir2.y));
        float weight  (dot_prod(dir, normal));
        if (weight > 0)
            r[d].add(voxel_raycast(origin, origin + dir * raylen), weight);
    }
}

sun_lightmap::rays
sun_lightmap::generate (float raylen, size_t count) const
{
    rays result;

    switch (count)
    {
        case 0:
            add(result, raylen, direction_);
            // Fallthrough

        case 1:
            for (int i (0); i < 6; ++i)
            {
                double a (((2.* 3.1415827) / 6.) * i);
                vector2<float> r (sin(a), cos(a));
                vector2<float> dir2 (direction_ + r * radius_);
                add(result, raylen, dir2);
            }
            // Fallthrough

        case 2:
            for (int i (0); i < 6; ++i)
            {
                double a (((2.* 3.1415827) / 6.) * float(i + .5));
                vector2<float> r (sin(a), cos(a));
                vector2<float> dir2 (direction_ + r * radius_ * 0.5);
                add(result, raylen, dir2);
            }
            break;

        default:
            assert(false);
    }

    // Normalize all rays so the highest intensity is 1.0
    float max (0);
    for (auto& r : result)
    {
        if (r.weight > max)
            max = r.weight;
    }

    for (auto& r : result)
        r.multiply_weight(1.0f / max);

    return result;
}

float
sun_lightmap::recurse (const ray_bundle& r, float ray_power,
                       const world_coordinates& blk,
                       world_lightmap_access& data,
                       bool first) const
{
    float temp (0.0f);
    bool should_recurse (true);
    for (auto& voxel : r.trunk)
    {
        auto type (data[blk + voxel].type);

        // If the very first block we traverse is a custom block, we
        // skip it.
        if (first)
        {
            first = false;
            if (material_prop[type].is_custom_block())
                continue;
        }

        temp += opacity(type);
        if (temp >= 1.0f)
        {
            should_recurse = false;
            break;
        }
    }

    ray_power -= std::min(temp, 1.0f) * r.weight;

    if (ray_power <= 0.01)
        return 0.0;

    if (should_recurse)
    {
        for (auto& s : r.branches)
            ray_power = recurse(s, ray_power, blk, data, first);
    }

    return ray_power;
}

lightmap&
sun_lightmap::generate (world_lightmap_access& data,
                        const chunk_coordinates& pos,
                        const surface& s,
                        lightmap& lightchunk,
                        unsigned int phase) const
{
    trace((boost::format("for %1%") % world_vector(pos - world_chunk_center)).str());

    auto lmi (std::begin(lightchunk));

    for (faces f : s)
    {
        world_coordinates blk (pos * chunk_size + f.pos);

        for (int d (0); d < 6; ++d)
        {
            if (!f[d])
                continue;

            const ray_bundle& r (detail_levels_[phase][d]);
            float light_level (recurse(r, r.weight, blk, data));
            lmi->sunlight = clamp(light_level, 0.0f, 1.0f) * 15.4f;
            ++lmi;
        }
    }
    assert(lmi == std::end(lightchunk));
    trace((boost::format("done with %1%") % world_vector(pos - world_chunk_center)).str());

    return lightchunk;
}

} // namespace hexa

