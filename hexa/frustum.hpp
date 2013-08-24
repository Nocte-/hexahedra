//---------------------------------------------------------------------------
/// \file   hexa/frustum.hpp
/// \brief  A view frustum class.
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

#include <array>
#include "plane3d.hpp"
#include "vector3.hpp"
#include "matrix.hpp"

namespace hexa {

/** View frustum. */
class frustum : public std::array<plane3d<float>, 6>
{
public:
    /** Create a view frustum from a projection matrix. */
    frustum(const matrix4<float>& mtx);

    /** Check if a sphere is inside the frustum. */
    inline
    bool is_inside (const vector3<float>& center, float radius = 0.0f) const
    {
        return    distance((*this)[0], center) < radius
               && distance((*this)[1], center) < radius
               && distance((*this)[2], center) < radius
               && distance((*this)[3], center) < radius
               && distance((*this)[4], center) < radius;
    }
};

} // namespace hexa

