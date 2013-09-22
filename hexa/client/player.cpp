//---------------------------------------------------------------------------
// client/player.cpp
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

#include "player.hpp"

#include <cassert>
#include <iostream>
#include <boost/math/constants/constants.hpp>

#include <hexa/block_types.hpp>

using namespace boost::math::constants;

namespace hexa {

player::player()
    : position_ (0,0,0)
    , position_fraction_ (0.5f, 0.5f, 0.5f)
    , look_at_ (0.0f, pi<float>() * 0.5f)
    , height_ (1.7f)
    , velocity (0,0,0)
    , active_slot (0)
    , hotbar_needs_update (false)
    , is_airborne (true)
{ }

void player::move_to(const world_coordinates& pos, const vector& fraction)
{
    position_ = pos;
    position_fraction_ = fraction;
}

void player::look_at(const yaw_pitch& direction)
{
    look_at_ = direction;
}

yaw_pitch player::turn_head(const yaw_pitch& amount)
{
    look_at_ += amount;
    if (look_at_.x >  pi<float>()) look_at_.x -= 2. * pi<float>();
    if (look_at_.x < -pi<float>()) look_at_.x += 2. * pi<float>();

    float b (0.01f);
    if (look_at_.y > pi<float>() - b) look_at_.y = pi<float>() - b;
    if (look_at_.y < 0.0f + b       ) look_at_.y = 0.0f + b;

    return look_at_;
}

void player::move(const vector& delta)
{
    vector no_z (delta.x, delta.y, 0);

    position_fraction_ += delta; // no_z;
    vector3<int> quantize (floor(position_fraction_));
    position_fraction_ -= quantize;

    assert(position_fraction_.x >= 0.f && position_fraction_.x < 1.f);
    assert(position_fraction_.y >= 0.f && position_fraction_.y < 1.f);
    position_ += quantize;
}

yaw_pitch player::head_angle() const
{
    return look_at_;
}

yaw_pitch player::walk_angle() const
{
    yaw_pitch result (look_at_);
    result.y = 0;

    return result;
}

float player::height() const
{
    return height_;
}

} // namespace hexa

