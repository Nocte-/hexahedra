//---------------------------------------------------------------------------
// server/light/radiosity_lightmap.cpp
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
// Copyright 2013-2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "radiosity_lightmap.hpp"

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

namespace hexa
{

radiosity_lightmap::radiosity_lightmap(world& c,
                                       const property_tree::ptree& conf)
    : lightmap_generator_i(c, conf)
{
}

radiosity_lightmap::~radiosity_lightmap()
{
}

struct radiosity
{
    radiosity(chunk_index p, float s)
        : pos(p)
        , str(s)
    {
    }

    bool operator==(chunk_index p) const { return pos == p; }

    bool operator==(const radiosity& p) const { return pos == p.pos; }

    bool operator<(const radiosity& p) const { return pos < p.pos; }

    chunk_index pos;
    float str;
};

void radiosity_lightmap::generate(world_lightmap_access& data,
                                       const chunk_coordinates& pos,
                                       const surface& s, lightmap_hr &lightchunk,
                                       unsigned int phase) const
{
    std::array<float, 6> irr_sun, irr_ambient, irr_artificial;
    vector half {0.5f, 0.5f, 0.5f};
    auto lmi = std::begin(lightchunk);
    for (const faces& f : s)
    {
        fill(irr_sun, 0.f);
        fill(irr_ambient, 0.f);
        fill(irr_artificial, 0.f);

        auto surr (surroundings(f.pos, phase + 1));
        for (auto sp : surr)
        {
            if (sp == f.pos)
                continue;

            auto found (find(s, sp));
            if (found == s.end())
                continue;

            auto& other (*found);
            for (int i = 0; i < 6; ++i)
            {
                if (!f[i])
                    continue;

                vector this_face (half + f.pos + vector(dir_vector[i]) * 0.52f);
                for (int j = 0; j < 6; ++j)
                {
                    if (i == j || !other[j])
                        continue;

                    vector that_face (half + other.pos + vector(dir_vector[j])* 0.52f);
                    vector conn (that_face - this_face);
                    vector norm_conn (normalize(conn));
                    float dp1 (dot_prod<vector>(dir_vector[j], -norm_conn));
                    if (dp1 <= 0)
                        continue;

                    float dp2 = dot_prod<vector>(dir_vector[i], norm_conn);
                    if (dp2 <= 0)
                        continue;

                    float intensity = dp1 * dp2 / squared_length(conn);

                    auto dist = std::distance(std::begin(s), found);
                    auto& olm = lightchunk.data[dist];

                    irr_sun[i] += olm.sunlight * intensity;
                    irr_ambient[i] += olm.ambient * intensity;
                    irr_artificial[i] += olm.artificial * intensity;
                }
            }
        }

        for (int i (0); i < 6; ++i)
        {
            if (!f[i])
                continue;

            lmi->r_sunlight = irr_sun[i] * 255.f + 0.49f;
            lmi->r_ambient = irr_ambient[i] * 255.f + 0.49f;
            lmi->r_artificial = irr_artificial[i] * 255.f + 0.49f;

            ++lmi;
        }
    }
}

} // namespace hexa
