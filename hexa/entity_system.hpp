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
// Copyright 2013-2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include <es/storage.hpp>

#include "serialize.hpp"
#include "hotbar_slot.hpp"
#include "ip_address.hpp"
#include "vector2.hpp"
#include "vector3.hpp"
#include "wfpos.hpp"
#include "read_write_lockable.hpp"

#include "server/random.hpp"

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
        c_boundingbox,  // vector       Size of the AABB
        c_impact,       // vector       Impact vector after collision
        c_model,        // uint16       3D model
        c_name,         // std::string  Name
        c_lookat,       // yaw_pitch    Angle of the head
        c_lag_comp,     // last_known_phys
        c_hotbar,       // hotbar       Player's hotbar
        c_last_component // Used in server/entities.hpp
    };

    void set_position (es::entity e, const wfpos& p)
        { set(e, c_position, p); }

    void set_position (es::storage::iterator e, const wfpos& p)
        { set(e, c_position, p); }

    void set_velocity (es::entity e, const vector& v)
        { set(e, c_velocity, v); }

    void set_velocity (es::storage::iterator e, const vector& v)
        { set(e, c_velocity, v); }

    void set_force (es::entity e, const vector& v)
        { set(e, c_force, v); }

    void set_walk (es::entity e, const vector2<float>& v)
        { set(e, c_walk, v); }

    void set_orientation (es::entity e, float v)
        { set(e, c_orientation, v); }

    void set_orientation (es::storage::iterator e, float v)
        { set(e, c_orientation, v); }

    void set_boundingbox (es::entity e, const vector& v)
        { set(e, c_boundingbox, v); }

    void set_impact (es::entity e, const vector& v)
        { set(e, c_impact, v); }

    void set_impact (es::storage::iterator e, const vector& v)
        { set(e, c_impact, v); }

    void set_model (es::entity e, uint16_t v)
        { set(e, c_model, v); }

    void set_model (es::storage::iterator e, uint16_t v)
        { set(e, c_model, v); }

    void set_name (es::entity e, const std::string& v)
        { set(e, c_name, v); }

    void set_name (es::storage::iterator e, const std::string& v)
        { set(e, c_name, v); }

    void set_lookat (es::entity e, yaw_pitch v)
        { set(e, c_lookat, v); }

    void set_lag_comp (es::entity e, const last_known_phys& v)
        { set(e, c_lag_comp, v); }

    void set_hotbar (es::entity e, const hotbar& v)
        { set(e, c_hotbar, v); }

    void set_hotbar (es::storage::iterator e, const hotbar& v)
        { set(e, c_hotbar, v); }

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
struct is_flat<hexa::yaw_pitch>  { static constexpr bool value = true; };

template <>
struct is_flat<hexa::ip_address> { static constexpr bool value = true; };

template <>
struct is_flat<hexa::last_known_phys> { static constexpr bool value = true; };


template<> inline
void
serialize<hexa::hotbar>(const hexa::hotbar& hb, std::vector<char>& buf)
{
    hexa::make_serializer(buf)(hb);
}

template<> inline
std::vector<char>::const_iterator
deserialize<hexa::hotbar>(hexa::hotbar& hb,
                          std::vector<char>::const_iterator first,
                          std::vector<char>::const_iterator last)
{
    return first + hexa::make_deserializer(first, last)(hb).bytes_read();
}

} // namespace es

