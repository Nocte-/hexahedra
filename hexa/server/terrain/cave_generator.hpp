//---------------------------------------------------------------------------
/// \file   server/terrain/cave_generator.hpp
/// \brief  Carve out cave systems.
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

#include <memory>
#include "terrain_generator_i.hpp"

namespace hexa
{

/** Generates random, sprawling caverns */
class cave_generator : public terrain_generator_i
{
    struct impl;
    std::unique_ptr<impl> pimpl_;

public:
    /** Constructor
     *  @param w The game world
     *  @param conf  The following properties are used:
     */
    cave_generator(world& w, const boost::property_tree::ptree& conf);

    virtual ~cave_generator();

    void generate(world_terraingen_access& data, const chunk_coordinates& pos,
                  chunk& cnk) override;
};

} // namespace hexa
