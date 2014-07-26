//---------------------------------------------------------------------------
// server/lilght/test_lightmap.cpp
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

#include "test_lightmap.hpp"

#include <hexa/lightmap.hpp>

#include "../world.hpp"
#include "../world_lightmap_access.hpp"

using namespace boost::property_tree;

namespace hexa
{

test_lightmap::test_lightmap(world& c, const ptree& conf)
    : lightmap_generator_i(c, conf)
{
}

test_lightmap::~test_lightmap()
{
}

lightmap& test_lightmap::generate(world_lightmap_access&,
                                  const chunk_coordinates&, const surface& s,
                                  lightmap& lc, unsigned int) const
{
    auto lmi(std::begin(lc));

    for (auto& face : s) {
        for (int d = 0; d < 6; ++d) {
            if (face[d]) {
                lmi->sunlight = face.pos.x;
                lmi->ambient = face.pos.y;
                ++lmi;
            }
        }
    }

    return lc;
}

} // namespace hexa
