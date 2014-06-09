//---------------------------------------------------------------------------
// client/camera.cpp
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

#include "camera.hpp"

#include <boost/math/constants/constants.hpp>

using namespace boost::math::constants;

namespace hexa
{

camera::camera(vector pos, yaw_pitch look_dir, float roll, float fov,
               float aspect_ratio, float near, float far)
    : pos_(pos)
    , fov_(fov)
    , aspect_ratio_(aspect_ratio)
    , near_(near)
    , far_(far)
    , modelview_matrix_(matrix4<float>::identity())
    , projection_matrix_(matrix4<float>::identity())
    , mvp_matrix_(matrix4<float>::identity())
{
    typedef quaternion<float> q;

    orientation_ = q::from_angle_and_axis(roll, vector(0, 1, 0))
                   * q::from_angle_and_axis(look_dir.y - 0.5f * pi<float>(),
                                            vector(1, 0, 0))
                   * q::from_angle_and_axis(look_dir.x, vector(0, 0, 1));

    projection_matrix_(3, 2) = -1.0f;
    projection_matrix_(3, 3) = 0.0f;

    recalc_mvm();
    recalc_pm();
}

void camera::move(const vector& motion)
{
    pos_ += motion;
    move_to(pos_);
}

void camera::move_to(const vector& pos)
{
    pos_ = pos;

    modelview_matrix_(0, 3) = -pos.x;
    modelview_matrix_(1, 3) = -pos.y;
    modelview_matrix_(2, 3) = -pos.z;

    mvp_matrix_ = projection_matrix_ * modelview_matrix_;

    recalc_mvm();
}

void camera::rotate(double angle, const vector& axis)
{
    orientation_.rotate(-angle, axis);
    recalc_mvm();
}

void camera::rotate(const quaternion<float>& rotation)
{
    orientation_ *= ~rotation;
    recalc_mvm();
}

void camera::look_at(const vector& point)
{
    ///\todo
}

void camera::recalc_mvm()
{
    modelview_matrix_ = matrix4<float>(rotation_matrix(orientation_))
                        * matrix4<float>::translation(-pos_);

    mvp_matrix_ = projection_matrix_ * modelview_matrix_;
}

void camera::recalc_pm()
{
    float f(1.0f / (std::tan(fov_ * 0.5f)));

    static const matrix4<float> flip(1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0,
                                     0, 1);

    projection_matrix_(0, 0) = f / aspect_ratio_;
    projection_matrix_(1, 1) = f;
    projection_matrix_(2, 2) = (far_ + near_) / (near_ - far_);
    projection_matrix_(2, 3) = (2.0f * far_ * near_) / (near_ - far_);

    projection_matrix_ = projection_matrix_ * flip;
    mvp_matrix_ = projection_matrix_ * modelview_matrix_;
}

} // namespace hexa
