//---------------------------------------------------------------------------
/// \file   server/gridworld_generator.hpp
/// \brief  Generate a completely grid world with a cube in the middle.
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

/** This generator will create an endless 3-D grid. */
class gridworld_generator : public terrain_generator_i
{
public:
    gridworld_generator (world& w, const boost::property_tree::ptree& conf)
        : terrain_generator_i (w, conf) {}

    void generate (chunk_coordinates pos, chunk& dest);
};

} // namespace hexa

