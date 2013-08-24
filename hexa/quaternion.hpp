//---------------------------------------------------------------------------
/// \file   quaternion.hpp
/// \brief  Quaternion class
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

#include <cassert>
#include <cmath>
#include "vector3.hpp"
#include "matrix.hpp"

namespace hexa {

/** A quaternion. */
template <typename type>
class quaternion : public vector4<type>
{
    typedef vector4<type>       base;

public:
    typedef type                value_type;
    typedef quaternion<type>    self;

    using base::x;
    using base::y;
    using base::z;
    using base::w;

    using base::operator=;
    using base::operator==;
    using base::operator!=;
    using base::operator+=;
    using base::operator-=;
    using base::operator*=;
    using base::operator/=;

public:
    /** Construct a scalar quaternion. */
    quaternion (type w = 1)
        : base (0, 0, 0, w)
    { }

    /** Construct a unit quaternion from a given vector. */
    quaternion (type x, type y, type z)
        : base (x, y, z, 0)
    {
        normalize_w();
    }

    /** Construct a quaternion from the given scalar and vector parts. */
    quaternion (type x, type y, type z, type w)
        : base (x, y, z, w)
    { }

    /** Construct a unit quaternion from a given vector. */
    quaternion (const vector3<type>& xyz)
        : base (xyz, 0)
    {
        normalize_w();
    }

    /** Construct a quaternion from the given scalar and vector parts. */
    quaternion (const vector3<type>& xyz, type w)
        : base (xyz, w)
    { }

    /** Construct a quaternion from a C-style array [x,y,z,w]. */
    explicit quaternion (const value_type* ptr)
        : base (ptr)
    { }

    quaternion (const vector4<type>& xyzw)
        : base (xyzw)
    { }

    /** Make a quaternion that models a rotation around a given axis.
     * @param angle  Rotation angle in radians.
     * @param axis   The axis to rotate about as a normalized vector. */
    static quaternion<type>
        from_angle_and_axis (double angle, const vector3<type>& axis)
    {
        assert(normalize(axis) == axis);
        double half (angle * 0.5);
        return self(axis * std::sin(half), std::cos(half));
    }

    const type scalar_part() const
    {
        return w;
    }

    type& scalar_part()
    {
        return w;
    }

    const vector3<type> vector_part() const
    {
        return vector3<type>(x, y, z);
    }

    /** Rotate the quaterion around an axis by a given angle.
     * @param angle The angle in radians
     * @param axis  The normalized vector of the axis
     * @return The rotated quaternion */
    self& rotate (double angle, const vector3<type>& axis)
    {
        assert(normalize(axis) == axis);
        return *this *= self::from_angle_and_axis(angle, axis);
    }

    void normalize_w()
    {
        type t (type(1) - x*x - y*y - z*z);
        if (t <= 0)
            w = 0;
        else
            w = std::sqrt(t);
    }

public:
    self& operator*= (const self& b)
    {
        return *this = self(
            w * b.x + x * b.w + y * b.z - z * b.y,
            w * b.y + y * b.w + z * b.x - x * b.z,
            w * b.z + z * b.w + x * b.y - y * b.x,
            w * b.w - x * b.x - y * b.y - z * b.z
        );
    }

    self& operator+= (type sc)
    {
        w += sc;
        return *this;
    }

    self& operator-= (type sc)
    {
        w -= sc;
        return *this;
    }

    /** Returns the conjugate. */
    self operator~() const
    {
        return self(-x, -y, -z, w);
    }
};

//---------------------------------------------------------------------------

template <typename type>
const quaternion<type> operator+ (quaternion<type> a, const quaternion<type>& b)
{
    return a += b;
}

template <typename type>
const quaternion<type> operator+ (quaternion<type> a, type b)
{
    return a += b;
}

template <typename type>
const quaternion<type> operator+ (type b, quaternion<type> a)
{
    return a += b;
}

template <typename type>
const quaternion<type> operator- (quaternion<type> a, const quaternion<type>& b)
{
    return a -= b;
}

template <typename type>
const quaternion<type> operator- (quaternion<type> a, type b)
{
    return a -= b;
}

template <typename type>
const quaternion<type> operator- (type b, quaternion<type> a)
{
    return a -= b;
}

template <typename type>
const quaternion<type> operator* (quaternion<type> a, type b)
{
    return a *= b;
}

template <typename type>
const quaternion<type> operator* (type b, quaternion<type> a)
{
    return a *= b;
}

template <typename type>
const quaternion<type> operator* (quaternion<type> a, const quaternion<type>& b)
{
    return a *= b;
}

template <typename type>
const quaternion<type> operator/ (quaternion<type> a, type b)
{
    return a /= b;
}

//---------------------------------------------------------------------------

template <typename type>
const vector3<type> operator* (const quaternion<type>& rot, const vector3<type>& v)
{
    vector3<type> uv  (cross_prod(rot.vector_part(), v ));
    vector3<type> uuv (cross_prod(rot.vector_part(), uv));

    uv *= rot.scalar_part() * type(2);
    uuv *= type(2);

    return v + uv + uuv;
}

//---------------------------------------------------------------------------

template <typename type>
quaternion<type> inverse (const quaternion<type>& q)
{
    return ~q / squared_length(q);
}

/** Spherical linear interpolation between two rotations. */
template <typename type>
quaternion<type> slerp (const quaternion<type>& a, const quaternion<type>& b,
                        double amount)
{
    double cosval (dot_prod(a, b));

    // If a and b are colinear, just return one of them.
    if (cosval < -1 || cosval > 1)
        return a;

    // Try to interpolate over the shortest path between the quaternions.
    double factor (1);
    if (cosval < 0)
    {
        factor = -1;
        cosval = -cosval;
    }

    double theta (std::acos(cosval));
    double sin_theta (std::sin(theta));

    if (sin_theta > 0.00001)
    {
        type w1 (std::sin((1.0 - amount) * theta) / sin_theta);
        type w2 (std::sin(       amount  * theta) / sin_theta * factor);
        return a * w1 + b * w2;
    }

    return lerp(a, b, amount);
}

/** Make a quaternion from Euler angles. */
template <typename type>
quaternion<type> from_euler_angles (float rx, float ry, float rz)
{
    quaternion<type> qx (1, 0, 0, -rx), qy (0, 1, 0, -ry), qz (0, 0, 1, -rz);
    return qx * qy * qz;
}

/** Make a rotation matrix from a quaternion. */
template <typename type>
matrix3<type> rotation_matrix (const quaternion<type>& q)
{
    const type w (q.w), x (q.x), y (q.y), z (q.z);
    const type x2 (x + x), y2 (y + y), z2 (z + z);

    const type xx (x2 * x), xw (x2 * w), xy (x2 * y), xz (x2 * z),
               yy (y2 * y), yw (y2 * w), yz (y2 * z),
               zw (z2 * w), zz (z2 * z);

    matrix3<type> mtx;

    mtx(0,0) = type(1) - (yy + zz);
    mtx(0,1) = xy - zw;
    mtx(0,2) = xz + yw;

    mtx(1,0) = xy + zw;
    mtx(1,1) = type(1) - (xx + zz);
    mtx(1,2) = yz - xw;

    mtx(2,0) = xz - yw;
    mtx(2,1) = yz + xw;
    mtx(2,2) = type(1) - (xx + yy);

    return mtx;
}

//---------------------------------------------------------------------------

// For the lazy ;)

typedef quaternion<float>   quatf;
typedef quaternion<double>  quatd;

} // namespace hexa


//---------------------------------------------------------------------------

namespace std {

template <typename type>
ostream& operator<< (ostream& str, const hexa::quaternion<type>& q)
{
    return str << '(' << q.x << ',' << q.y << ',' << q.z << ','
               << q.w << ')';
}

} // namespace std

