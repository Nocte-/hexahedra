//---------------------------------------------------------------------------
/// \file   server/biome_generator.hpp
/// \brief  Basic biome based on Perlin noise.
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
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <memory>
#include "area_generator_i.hpp"

namespace hexa {

class biome_generator : public area_generator_i
{
    class impl;
    std::unique_ptr<impl> pimpl_;

public:
    biome_generator(world& w, const boost::property_tree::ptree& conf);
    virtual ~biome_generator();

    virtual area_data& generate (map_coordinates xy, area_data& dest);
};

} // namespace hexa

