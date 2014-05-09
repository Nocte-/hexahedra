//---------------------------------------------------------------------------
/// \file   server/terrain/object_placer.hpp
/// \brief  Place objects on the surface
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

#pragma once

#include "terrain_generator_i.hpp"
#include "../voxel_sprite.hpp"

namespace hexa {

class object_placer : public terrain_generator_i
{
public:
    object_placer(world& w,
                  const boost::property_tree::ptree& conf);

    void generate (world_terraingen_access& data,
                   const chunk_coordinates& pos,
                   chunk& cnk) override;

    chunk_height
    estimate_height (world_terraingen_access& data,
                     map_coordinates xy,
                     chunk_height prev) const override;

private:
    int density_map_;
    std::vector<voxel_sprite>   sprites_;
};

} // namespace hexa

