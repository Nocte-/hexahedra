//---------------------------------------------------------------------------
// server/light/uniform_lightmap.cpp
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

#include "uniform_lightmap.hpp"

#include <hexa/lightmap.hpp>

using namespace boost::property_tree;

namespace hexa
{

uniform_lightmap::uniform_lightmap(world& c, const ptree& conf)
    : lightmap_generator_i(c, conf)
    , sun_(conf.get<int>("sun", 255))
    , amb_(conf.get<int>("skydome", 255))
    , art_(conf.get<int>("artificial", 255))
    , sec_(conf.get<int>("secondary", 255))
{
}

uniform_lightmap::~uniform_lightmap()
{
}

void uniform_lightmap::generate(world_lightmap_access&,
                                     const chunk_coordinates&,
                                     const surface& s, lightmap_hr& lc,
                                     unsigned int) const
{
    chunk_index c1(0, 0, 0), c2(block_chunk_size);

    auto lmi(std::begin(lc));

    for (faces f : s) {
        for (int d(0); d < 6; ++d) {
            if (f[d]) {
                lmi->sunlight = sun_;
                lmi->ambient = amb_;
                lmi->artificial = art_;
                lmi->secondary = sec_;
                ++lmi;
            }
        }
    }
}

} // namespace hexa
