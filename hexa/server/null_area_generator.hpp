//---------------------------------------------------------------------------
/// \file   server/null_area_generator.hpp
/// \brief  Create empty areas.
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

#include <memory>
#include "area_generator_i.hpp"

namespace hexa {

class null_area_generator : public area_generator_i
{
public:
    null_area_generator(world& w, const boost::property_tree::ptree& conf)
        : area_generator_i(w, conf)
    { }

    ~null_area_generator() { }

    area_data& generate (map_coordinates, area_data& dest)
    {
        return dest;
    }
};

} // namespace hexa

