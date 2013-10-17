//---------------------------------------------------------------------------
/// \file   server/robinton_generator.hpp
/// \brief  Reads and converts terrain data from a "cubic chunk" save game
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
#include <boost/filesystem/path.hpp>
#include "area_generator_i.hpp"
#include "terrain_generator_i.hpp"

namespace hexa {

/** Minecraft world generator.
 *  This generator overlays the game world with terrain data read from a
 *  Minecraft save game. It only supports the new Beta "region" format. */
class robinton_generator : public terrain_generator_i
{
    struct impl;
    std::unique_ptr<impl> pimpl_;

public:
    /** Constructor.
     * @param w         The game world
     * @param savegame  The path to the Minecraft save game
     * @param origin    The center of the MC map will be aligned with this
     *                  position in the game world */
    robinton_generator(world& w, const boost::property_tree::ptree& conf);

    virtual ~robinton_generator();

    chunk_height
    estimate_height (map_coordinates xy, chunk_height prev) const;

    void
    generate (chunk_coordinates pos, chunk& dest);
};

} // namespace hexa

