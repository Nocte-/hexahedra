//---------------------------------------------------------------------------
/// \file   server/terrain/flatworld_generator.hpp
/// \brief  Generate a completely flat world.
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

#include "terrain_generator_i.hpp"

namespace hexa
{

/** This generator will create an endless, flat world. */
class flatworld_generator : public terrain_generator_i
{
public:
    flatworld_generator(world& w, const boost::property_tree::ptree& conf);

    void generate(world_terraingen_access& data, const chunk_coordinates& pos,
                  chunk& cnk) override;

    chunk_height estimate_height(world_terraingen_access&, map_coordinates,
                                 chunk_height) const override
    {
        return level_;
    }

    std::vector<std::string> area_generator() const override
    {
        return {"heightmap", "surface"};
    }

    bool generate(world_terraingen_access& data, const std::string& type,
                  map_coordinates pos, area_data& area) const override;

private:
    chunk_height level_;
    uint16_t block_type_;
};

} // namespace hexa
