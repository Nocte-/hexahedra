//---------------------------------------------------------------------------
// hexa/entity_system.cpp
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

#include "entity_system.hpp"

#include "basic_types.hpp"

namespace hexa
{

entity_system::entity_system()
{
    static_assert(es::is_flat<wfpos>::value,
                  "world_coordinates does not have a flat memory layout");
    static_assert(es::is_flat<vector>::value,
                  "vector does not have a flat memory layout");
    static_assert(es::is_flat<yaw_pitch>::value,
                  "yaw_pitch does not have a flat memory layout");

    register_component<wfpos>("position");
    register_component<vector>("velocity");
    register_component<vector>("force");
    register_component<vector2<float>>("walk");
    register_component<float>("orientation");
    register_component<vector>("boundingbox");
    register_component<vector>("impact");
    register_component<uint16_t>("model");
    register_component<std::string>("name");
    register_component<yaw_pitch>("lookat");
    register_component<last_known_phys>("lagcomp");
    auto check(register_component<hotbar>("hotbar"));

    if (check != c_hotbar)
        throw std::runtime_error("could not register component hotbar");
}

} // namespace hexa
