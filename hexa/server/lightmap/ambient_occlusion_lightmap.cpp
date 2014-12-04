//---------------------------------------------------------------------------
// server/light/ambient_occlusion_lightmap.cpp
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
// Copyright 2012-2014 nocte@hippie.nu
//---------------------------------------------------------------------------

#include "ambient_occlusion_lightmap.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <boost/format.hpp>
#include <boost/math/constants/constants.hpp>

#include <hexa/voxel_algorithm.hpp>
#include <hexa/voxel_range.hpp>
#include <hexa/trace.hpp>

#include "../world.hpp"
#include "../world_lightmap_access.hpp"

using namespace boost;
using namespace boost::property_tree;

namespace hexa
{

typedef std::array<unsigned int, 3> triangle;

namespace
{

float opacity(uint16_t t)
{
    return 1.0f - (material_prop[t].transparency / 255.f);
}

std::vector<vector> golden_spiral(int count)
{
    // Based on an implementation by Patrick Boucher.
    // http://www.softimage.blog/

    const float inv_phi = math::constants::pi<float>() * (3.0 - std::sqrt(5));

    std::vector<vector> result;
    float off(2.0f / count);
    for (int k = 0; k < count; ++k) {
        float z(k * off - 1.0f + (off / 2.0f));

        if (z <= 0)
            continue;

        float r(std::sqrt(1.0f - z * z));
        float th = k * inv_phi;
        result.emplace_back(std::cos(th) * r, std::sin(th) * r, z);
    }

    return result;
}

} // anonymous namespace

////////////////////////////////////////////////////////////////////////////

ambient_occlusion_lightmap::ambient_occlusion_lightmap(world& c,
                                                       const ptree& config)
    : lightmap_generator_i(c, config)
{
    detail_levels_.emplace_back(precalc(10, 10));
    detail_levels_.emplace_back(precalc(30, 40));
    detail_levels_.emplace_back(precalc(60, 100));
}

ambient_occlusion_lightmap::rays
ambient_occlusion_lightmap::precalc(float ambient_raylen,
                                    unsigned int count) const
{
    rays result;
    vector center{0.5f, 0.5f, 0.5f};

    for (auto v : golden_spiral(count)) {
        for (int i = 0; i < 5; ++i) {
            vector normal{dir_vector[i]};

            // Weight is calculated according to Lambert's cosine law:
            // intensity = cos theta = a . b (for unit vectors)
            float weight = dot_prod(v, normal);
            if (weight <= 0)
                continue;

            // The ray's origin is set to somewhat above the center of the
            // surface.  (Not too close: this darkens corners too much.)
            auto origin = center + normal * 0.8f;
            auto dir = v * ambient_raylen;
            result[i].add(voxel_raycast(origin, origin + dir), weight);
        }
    }

    float max = 0;
    for (int i = 0; i < 5; ++i) {
        if (result[i].weight > max)
            max = result[i].weight;
    }

    max = 1.0f / max;
    for (int i = 0; i < 5; ++i)
        result[i].multiply_weight(max);

    return result;
}

ambient_occlusion_lightmap::~ambient_occlusion_lightmap()
{
}

float ambient_occlusion_lightmap::recurse(const ray_bundle& r, float ray_power,
                                          const world_coordinates& blk,
                                          world_lightmap_access& data,
                                          bool first) const
{
    float temp = 0.0f;
    bool should_recurse = true;

    for (auto& voxel : r.trunk) {
        auto type = data[blk + voxel].type;

        // If the very first block we traverse is a custom block, we
        // skip it.
        if (first) {
            first = false;
            if (material_prop[type].is_custom_block())
                continue;
        }

        temp += opacity(type);
        if (temp >= 1.0f) {
            should_recurse = false;
            break;
        }
    }

    ray_power -= std::min(temp, 1.0f) * r.weight;

    if (ray_power <= 0.01)
        return 0.0;

    if (should_recurse) {
        for (auto& s : r.branches)
            ray_power = recurse(s, ray_power, blk, data, first);
    }

    return ray_power;
}

void ambient_occlusion_lightmap::generate(world_lightmap_access& data,
                                               const chunk_coordinates& pos,
                                               const surface& s,
                                               lightmap_hr& lightchunk,
                                               unsigned int phase) const
{
    assert(phase < detail_levels_.size());
    trace("for %1%", world_vector(pos - world_chunk_center));

    auto lmi = std::begin(lightchunk);
    for (faces f : s) {
        world_coordinates blk(pos * chunk_size + f.pos);

        for (int d = 0; d < 5; ++d) {
            if (f[d]) {
                const ray_bundle& r(detail_levels_[phase][d]);
                float light_level(recurse(r, r.weight, blk, data));

                light_level *= 1.0f + d * 0.05f;
                light_level = clamp(light_level, 0.0f, 1.0f);
                lmi->ambient = light_level * 255.0f + 0.49f;
                ++lmi;
            }
        }

        // Downward side is always dark.
        if (f[5]) {
            lmi->ambient = 0;
            ++lmi;
        }
    }
    assert(lmi == std::end(lightchunk));
    trace("done with %1%", world_vector(pos - world_chunk_center));
}

} // namespace hexa
