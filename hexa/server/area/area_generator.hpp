//---------------------------------------------------------------------------
/// \file   server/area/area_generator.hpp
/// \brief  Area generator module based on NHDL
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
#include <hexanoise/generator_context.hpp>
#include <hexanoise/node.hpp>
#include "area_generator_i.hpp"

namespace hexa {

class world;

/** Area generator that executes NHDL scripts. */
class area_generator : public area_generator_i
{
public:
    enum type
    {
        /** Default type; the result of the script is rounded and copied
         ** directly to the area data. */
        regular,

        /** The range -1..1 is mapped to -32767..32767 */
        normalized
    };

public:
    area_generator (world& w, const boost::property_tree::ptree& conf,
                    const noise::generator_context& ctx);

    area_data generate (map_coordinates) override;

private:
    const noise::generator_context&     ctx_;
    noise::node                         script_;
    std::unique_ptr<noise::generator_i> gen_;
    type                                type_;
};

} // namespace hexa

