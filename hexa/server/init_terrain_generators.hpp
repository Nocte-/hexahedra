//---------------------------------------------------------------------------
/// \file   server/init_terrain_generators.hpp
/// \brief  Chain the terrain generators together as specified in a config
///         file.
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
#pragma once

#include <boost/property_tree/ptree.hpp>
#include <hexanoise/generator_context.hpp>

namespace hexa
{

class world;

/** Terrain generator factory.
 *  This function chains terrain generators together, as specified by the
 *  configuration file. */
void init_terrain_gen(world& w, const boost::property_tree::ptree& config,
                      noise::generator_context& gen_context);

} // namespace hexa
