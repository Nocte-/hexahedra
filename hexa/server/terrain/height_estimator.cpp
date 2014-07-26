//---------------------------------------------------------------------------
// server/terrain/height_estimator.cpp
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

#include "height_estimator.hpp"

#include "../hndl.hpp"

namespace hexa
{

height_estimator::height_estimator(world& w,
                                   const boost::property_tree::ptree& conf)
    : terrain_generator_i{w}
    , func_{compile_hndl(conf.get<std::string>("hndl"))}
{
}

chunk_height height_estimator::estimate_height(world_terraingen_access&,
                                               map_coordinates pos,
                                               chunk_height) const
{
    auto hm = hndl_area_int16(*func_, pos);
    uint32_t highest = world_center.z + *std::max_element(hm.begin(), hm.end());
    
    return (highest >> cnkshift) + 1;
}

} // namespace hexa
