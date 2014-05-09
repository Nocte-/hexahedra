//---------------------------------------------------------------------------
// server/world_terraingen_access.hpp
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

#include "world_terraingen_access.hpp"

#include "world.hpp"

namespace hexa {

world_terraingen_access::world_terraingen_access (world& w)
    : w_(w)
{ }

world_terraingen_access::~world_terraingen_access()
{ }

const area_data&
world_terraingen_access::get_area_data (const map_coordinates& pos, uint16_t index)
{
    return w_.get_area_data(pos, index);
}

area_data&
world_terraingen_access::get_area_data_writable (const map_coordinates& pos, uint16_t index)
{
    return w_.get_area_data_writable(pos, index);
}

boost::optional<const area_data&>
world_terraingen_access::try_get_area_data (const map_coordinates& pos, int index)
{
    if (   index < 0
        || static_cast<size_t>(index) > w_.nr_of_registered_area_generators())
    {
        return boost::optional<const area_data&>();
    }
    return w_.get_area_data(pos, index);
}

boost::optional<area_data&>
world_terraingen_access::try_get_area_data_writable (const map_coordinates& pos, int index)
{
    if (   index < 0
        || static_cast<size_t>(index) > w_.nr_of_registered_area_generators())
    {
        return boost::optional<area_data&>();
    }
    return w_.get_area_data_writable(pos, index);
}


} // namespace hexa

