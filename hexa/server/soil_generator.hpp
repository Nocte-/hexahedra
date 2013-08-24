//---------------------------------------------------------------------------
/// \file   server/soil_generator.hpp
/// \brief  Cover the earth with grass and dirt
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
// Copyright 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include "terrain_generator_i.hpp"

namespace hexa {

class soil_generator : public terrain_generator_i
{
public:
    soil_generator(world& w,
                      const boost::property_tree::ptree& conf);

    void generate (chunk_coordinates pos, chunk& dest);

private:
    uint16_t    surfacemap_;
    uint16_t    grass_;
    uint16_t    dirt_;
    uint16_t    rock_;
    uint16_t    sand_;
};

} // namespace hexa

