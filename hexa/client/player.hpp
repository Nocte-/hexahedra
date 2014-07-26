//---------------------------------------------------------------------------
/// \file   client/player.hpp
/// \brief  Player state and movement.
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
// Copyright 2012-2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <hexa/basic_types.hpp>
#include <hexa/aabb.hpp>
#include <hexa/protocol.hpp>
#include <hexa/wfpos.hpp>

namespace hexa
{

/** The player.
 *  Since this is used at the client side, there's usually only one
 *  instance of this class. */
class player
{
public:
    player();

    void move_to(const world_coordinates& pos, const vector& fraction);

public:
    void look_at(const yaw_pitch& direction);
    yaw_pitch turn_head(const yaw_pitch& amount);
    void move(const vector& delta);
    void move_to(const wfpos& p);

public:
    yaw_pitch head_angle() const;
    yaw_pitch walk_angle() const;
    float height() const;

    wfpos get_wfpos() const { return wfpos(position_, position_fraction_); }

    world_coordinates position() const { return position_; }

    vector position_fraction() const { return position_fraction_; }

    vector3<double> world_position() const
    {
        return position() + position_fraction();
    }

    vector rel_world_position(const world_coordinates& o) const
    {
        return vector(world_vector(position() - o)) + position_fraction();
    }

    chunk_coordinates chunk_position() const { return position_ / chunk_size; }

    vector position_relative_in_chunk() const
    {
        return position_fraction() + vector(position() % chunk_size);
    }

    aabb<vector> collision_box() const
    {
        auto o(position_relative_in_chunk());
        return aabb<vector>(o - vector(0.4f, 0.4f, 0.01f),
                            o + vector(0.4f, 0.4f, 1.73f));
    }

private:
    void on_move();

private:
    world_coordinates position_;
    vector position_fraction_;
    yaw_pitch look_at_;
    vector2<float> walk_vector_;
    vector2<float> target_vector_;
    float height_;

public:
    vector velocity;
    unsigned int active_slot;

    typedef msg::player_configure_hotbar::slot slot;
    std::vector<slot> hotbar;
    bool hotbar_needs_update;
    bool is_airborne;
};

} // namespace hexa
