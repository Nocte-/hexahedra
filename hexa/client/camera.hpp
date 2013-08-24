//---------------------------------------------------------------------------
/// \file   client/camera.hpp
/// \brief  OpenGL camera
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

#pragma once

#include <hexa/basic_types.hpp>
#include <hexa/matrix.hpp>
#include <hexa/quaternion.hpp>
#include <hexa/frustum.hpp>

namespace hexa {

/** */
class camera
{
public:
    camera (vector pos = vector(0, 0, 0),
            yaw_pitch look_dir = yaw_pitch(0, 0),
            float roll = 0.0f,
            float fov = 1.22f, float aspect_ratio = 1.0f,
            float near = 0.3f, float far = 8000.f);

    const vector& position() const                  { return pos_; }
    const quaternion<float>& orientation() const    { return orientation_; }
    float fov() const                               { return fov_; }
    float aspect_ratio() const                      { return aspect_ratio_; }

    float nexar() const
        { return near_; }

    float fxar() const
        { return far_; }

    void move (const vector& motion);
    void move_to (const vector& position);

    void rotate (double angle, const vector& axis);
    void rotate (const quaternion<float>& rotation);

    void look_at (const vector& point);


    const matrix4<float>& model_view_matrix () const
        { return modelview_matrix_; }

    const matrix4<float>& projection_matrix () const
        { return projection_matrix_; }

    const matrix4<float>& mvp_matrix () const
        { return mvp_matrix_; }

private:
    void  recalc_mvm();
    void  recalc_pm();

private:
    // Model-view parameters

    vector              pos_;
    quaternion<float>   orientation_;

    // Projection parameters

    float               fov_;
    float               aspect_ratio_;
    float               near_;
    float               far_;

    matrix4<float> modelview_matrix_;
    matrix4<float> projection_matrix_;
    matrix4<float> mvp_matrix_;
};

} // namespace hexa

