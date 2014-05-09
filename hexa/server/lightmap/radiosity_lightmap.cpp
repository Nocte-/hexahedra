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

namespace hexa {

radiosity_lightmap::radiosity_lightmap
                (world& c, const property_tree::ptree& conf)
    : lightmap_generator_i (c, conf)
{
}

radiosity_lightmap::~radiosity_lightmap ()
{ }

struct radiosity
{
    radiosity(chunk_index p, float s) : pos(p), str(s) {}

    bool operator==(chunk_index p) const { return pos == p; }

    bool operator==(const radiosity& p) const { return pos == p.pos; }

    bool operator< (const radiosity& p) const { return pos <  p.pos; }

    chunk_index pos;
    float       str;
};

lightmap&
radiosity_lightmap::generate (world_lightmap_access& data,
                              const chunk_coordinates& pos,
                              const surface& s,
                              lightmap& lightchunk, unsigned int phase) const
{
    /*
    neighborhood<chunk_ptr> nbh (cache_, pos);

    std::array<float, 6> irr_sun, irr_ambient, irr_artificial;
    vector half (0.5f, 0.5f, 0.5f);
    auto lmi (std::begin(lightchunk));
    for (const faces& f : s)
    {
        fill(irr_sun, 0.f);
        fill(irr_ambient, 0.f);
        fill(irr_artificial, 0.f);

        auto surr (surroundings(f.pos, 2));
        for (auto sp : surr)
        {
            if (sp == f.pos)
                continue;

            auto found (find(s, sp));
            if (found == s.end())
                continue;

            auto& other (*found);
            for (int i (0); i < 6; ++i)
            {
                if (!f[i])
                    continue;

                vector this_face (half + f.pos + vector(dir_vector[i]) * 0.52f);
                for (int j (0); j < 6; ++j)
                {
                    if (i == j || !other[j])
                        continue;

                    vector that_face (half + other.pos + vector(dir_vector[j]) * 0.52f);
                    vector conn (that_face - this_face);
                    vector norm_conn (normalize(conn));
                    float dp1 (dot_prod<vector>(dir_vector[j], -norm_conn));
                    if (dp1 <= 0)
                        continue;

                    float dp2 (dot_prod<vector>(dir_vector[i], norm_conn));
                    if (dp2 <= 0)
                        continue;

                    //std::cout << this_face << " --> " << that_face << std::endl;
                    //std::cout << f.pos << "," << i << " --> " << other.pos << "," << j << std::endl;
                    //std::cout << conn << "   " << dp1 << " " << dp2 << std::endl;
                    float intensity (dp1 * dp2 / squared_length(conn));
                    //std::cout << intensity << std::endl;

                    auto dist (std::distance(std::begin(s), found));
                    auto& olm (lightchunk.data[dist]);

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

            if (irr_sun[i] > 1)
            {
                //std::cout << "sun " << irr_sun[i] << std::endl;
                lmi->sunlight = std::max(1, std::min<int>(15, lmi->sunlight + irr_sun[i] / 10.f));
            }
            if (irr_ambient[i] > 1)
                lmi->ambient = std::max(1, std::min<int>(15, lmi->ambient + irr_ambient[i] / 10.f));

            ++lmi;
        }
    }
*/
    return lightchunk;
}

} // namespace hexa

