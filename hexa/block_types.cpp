//---------------------------------------------------------------------------
// lib/block_types.cpp
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
// Copyright 2011-2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "block_types.hpp"

#include <boost/range/algorithm.hpp>

using namespace boost::range;

namespace hexa
{

std::vector<material> material_prop;
std::unordered_map<std::string, uint16_t> texture_names;
std::unordered_map<std::string, custom_block> custom_blocks;

material& register_new_material(uint16_t type_id)
{
    if (type_id >= material_prop.size())
        material_prop.resize(65536); //(type_id + 1);

    return material_prop[type_id];
}

uint16_t find_material(const std::string& name, uint16_t default_material)
{
    auto found(find(material_prop, name));
    return found == material_prop.end()
               ? default_material
               : std::distance(material_prop.begin(), found);
}

namespace
{

struct setup_materials
{
    setup_materials()
    {
        material_prop.resize(256);
        material_prop[0].name = "air";
        material_prop[0].is_solid = false;
        material_prop[0].transparency = 255;
    }
};

static setup_materials init_;
}

} // namespace hexa
