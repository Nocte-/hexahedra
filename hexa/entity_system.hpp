//---------------------------------------------------------------------------
/// \file   hexa/entity_system.hpp
/// \brief  Setup of the Entity System
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

#include <es/storage.hpp>

#include "hotbar_slot.hpp"
#include "ip_address.hpp"
#include "vector2.hpp"
#include "vector3.hpp"
#include "wfpos.hpp"
#include "read_write_lockable.hpp"

namespace hexa {

struct last_known_phys
{
    wfpos   position;
    vector  speed;
};

class entity_system : public es::storage, public readers_writer_lock
{
public:
    enum basic_components
    {
        c_position = 0, // wfpos        Position in world-float coordinates
        c_velocity,     // vector       Current speed
        c_force,        // vector       Force exerted
        c_walk,         // vector2      Walk vector
        c_orientation,  // float
        c_boundingbox,  // vector
        c_impact,       // vector       Resulting impact vector after collision
        c_model,        // uint16       3D model
        c_name,         // std::string  Name
        c_lookat,       // yaw_pitch    Angle of the head
        c_lag_comp,     // last_known_phys
        c_hotbar,       // hotbar       Player's hotbar
        c_last_component // Used in server/entities.hpp
    };

public:
    entity_system();
};

} // namespace hexa

//---------------------------------------------------------------------------

// Inform ES about the memory layout of some classes we use

namespace es {

template <typename t>
struct is_flat<hexa::vector2<t>> { static constexpr bool value = true; };

template <typename t>
struct is_flat<hexa::vector3<t>> { static constexpr bool value = true; };

template <>
struct is_flat<hexa::wfpos>      { static constexpr bool value = true; };

template <>
struct is_flat<hexa::ip_address> { static constexpr bool value = true; };

template <>
struct is_flat<hexa::last_known_phys> { static constexpr bool value = true; };


} // namespace es

