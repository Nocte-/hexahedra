//---------------------------------------------------------------------------
// lib/frustum.cpp
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

#include "frustum.hpp"

namespace hexa {

frustum::frustum (const matrix4<float>& mtx)
{
    float planes[6][4];

    for (int i (0); i < 4; ++i)
    {
        planes[0][i] = mtx(3, i) + mtx(0, i); // Left
        planes[1][i] = mtx(3, i) - mtx(0, i); // Right
        planes[2][i] = mtx(3, i) + mtx(1, i); // Bottom
        planes[3][i] = mtx(3, i) - mtx(1, i); // Top
        planes[4][i] = mtx(3, i) + mtx(2, i); // Near
        planes[5][i] = mtx(3, i) - mtx(2, i); // Far
    }

    for (int i (0); i < 6; ++i)
    {
        vector t (planes[i]);
        const float inv_len (-1.f / length(t));

        (*this)[i].normal = t * inv_len;
        (*this)[i].distance = planes[i][3] * inv_len;
    }
}

} // namespace hexa

