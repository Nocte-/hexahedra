//---------------------------------------------------------------------------
/// \file   server/ocean_generator.hpp
/// \brief  Fill air blocks with water, up to a certain level
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

#include "terrain_generator_i.hpp"

namespace hexa {

/** This generator will fill air blocks with water, up to a certain level*/
class ocean_generator : public terrain_generator_i
{
public:
    ocean_generator (world& w, const boost::property_tree::ptree& conf);

    void generate (chunk_coordinates pos, chunk& dest);

    chunk_height estimate_height (map_coordinates, chunk_height) const
        { return 1 + ((level_ - 1) / chunk_size); }

private:
    void init();

private:
    uint16_t    material_;
    uint32_t    level_;
    uint16_t    heightmap_;
};

} // namespace hexa

