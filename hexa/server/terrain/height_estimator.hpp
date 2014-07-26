//---------------------------------------------------------------------------
/// \file   server/terrain/height_estimator.hpp
/// \brief  Use a function to estimate the terrain height.
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
#include <hexanoise/generator_i.hpp>
#include "terrain_generator_i.hpp"

namespace hexa
{

/** Estimate the height of the terrain with an HNDL function. */
class height_estimator : public terrain_generator_i
{
public:
    height_estimator(world& w, const boost::property_tree::ptree& conf);

    void generate(world_terraingen_access& data, const chunk_coordinates& pos,
                  chunk& cnk) override
    {
    }

    chunk_height estimate_height(world_terraingen_access&, map_coordinates pos,
                                 chunk_height) const override;

private:
    std::unique_ptr<noise::generator_i> func_;
};

} // namespace hexa
