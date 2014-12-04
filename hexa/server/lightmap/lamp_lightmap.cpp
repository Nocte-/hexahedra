//---------------------------------------------------------------------------
// server/light/lamp_lightmap.cpp
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

#include "lamp_lightmap.hpp"

#include <algorithm>
#include <array>
#include <unordered_set>
#include <iostream>

#include <boost/math/constants/constants.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptors.hpp>

#include <hexa/voxel_algorithm.hpp>
#include <hexa/voxel_range.hpp>

#include "../world.hpp"
#include "../world_lightmap_access.hpp"

using namespace boost;
using namespace boost::adaptors;
using namespace boost::range;
using namespace boost::property_tree;

namespace hexa
{

namespace
{

float opacity(uint16_t t)
{
    return 1.0f - (material_prop[t].transparency / 255.f);
}

} // anonymous namespace

////////////////////////////////////////////////////////////////////////////

lamp_lightmap::lamp_lightmap(world& c, const ptree& conf)
    : lightmap_generator_i(c, conf)
{
}

lamp_lightmap::~lamp_lightmap()
{
}

struct lamp
{
    lamp(vector p, float s)
        : pos(p + vector(0.5, 0.5, 0.5))
        , str(s)
    {
    }

    vector pos;
    float str;
};

void lamp_lightmap::generate(world_lightmap_access& data,
                                  const chunk_coordinates& pos,
                                  const surface& s, lightmap_hr& lightchunk,
                                  unsigned int phase) const
{
    static const float boost = 6.0f;

    if (s.empty())
        return;

    const vector half{0.5f, 0.5f, 0.5f};
    block_vector no{chunk_size * 2, chunk_size * 2, chunk_size * 2};

    // Go through all the surfaces in the chunks surrounding the current
    // chunk we're making a lightmap for (a 5x5x5 Moore neighborhood).
    // Put all the block types in a flat array (we'll need to access them
    // quite often, this speeds up the traversal) and place the point light
    // sources.
    //
    std::vector<lamp> lamps;
    chunk_base<block, chunk_size * 5> nbh;
    for (auto i : cube_range<block_vector>(2)) {
        auto& cp(data.get_surface(pos + i));

        block_vector origin(i * chunk_size + no);
        for (auto& face : cp.opaque) {
            block_vector pos(origin + face.pos);
            nbh[pos] = face.type;

            uint8_t strength(material_prop[face.type].light_emission);
            if (strength)
                lamps.push_back(lamp(pos - no, strength / 255.f));
        }
        for (auto& face : cp.transparent) {
            block_vector pos(origin + face.pos);
            nbh[face.pos] = face.type;

            uint8_t strength(material_prop[face.type].light_emission);
            if (strength)
                lamps.push_back(lamp(pos - no, strength / 255.f));
        }
    }

    if (lamps.empty())
        return;

    auto lmi(std::begin(lightchunk));
    for (const faces& f : s) {
        for (int d = 0; d < 6; ++d) {
            if (!f[d])
                continue;

            vector normal = dir_vector[d];
            vector o = vector{f.pos} + half + (normal * 0.51f);

            float light_level = 0.0f;

            for (auto& lamp : lamps) {
                const vector& lp = lamp.pos;
                auto ilp = floor(lp);

                if (ilp == f.pos) {
                    light_level = 1;
                    break;
                }

                float weight = lamp.str * dot_prod(normalize(lp - o), normal)
                              / squared_distance(lp, o);
                if (weight <= 0)
                    continue;

                float power(1.0f);
                // Follow the line from the middle of the face to the lamp.
                // Stop right before the lamp block is found.  Decrease the
                // light power for every block that is not completely
                // transparent.
                voxel_raycast(o, lp, [&](vector3<int> rv) {
                    return rv == ilp || (power -= opacity(nbh[rv + no])) <= 0;
                });

                if (power > 0) {
                    light_level += power * weight * boost;
                    if (light_level >= 1)
                        break;
                }
            }

            light_level = clamp(light_level, 0.0f, 1.0f);
            lmi->artificial = light_level * 255.0f + 0.49f;
            ++lmi;
        }
    }
    assert(lmi == std::end(lightchunk));
}

} // namespace hexa
