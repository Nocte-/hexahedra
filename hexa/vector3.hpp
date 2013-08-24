//---------------------------------------------------------------------------
/// \file   hexa/vector3.hpp
/// \brief  3-D vector class
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

#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include "algorithm.hpp"
#include "vector2.hpp"

namespace hexa {

// Forward declaration
template <typename type>
class vector2;

/** A 3-D vector. */
template <typename type>
class vector3
{
    typedef vector3<type>    self;

public:
    /** The x ordinate (increases east). */
    type    x;
    /** The y ordinate (increases north). */
    type    y;
    /** The z ordinate (increases up). */
    type    z;

    typedef type        value_type;

public:
    vector3 () {}

    vector3 (std::initializer_list<type> l)
    {
        assert(l.size() == 3);
        auto i (std::begin(l));
        x = *i++;
        y = *i++;
        z = *i;
    }

    vector3 (type init_x, type init_y, type init_z)
        : x (init_x), y (init_y), z (init_z)
    { }

    explicit vector3 (type scalar)
        : x (scalar), y (scalar), z (scalar)
    { }

    explicit vector3 (const type* ptr)
        : x (ptr[0]), y (ptr[1]), z (ptr[2])
    { }

    template <typename assign_t>
    vector3 (const vector3<assign_t>& init)
        : x (init.x), y (init.y), z (init.z)
    { }

    template <typename assign_t>
    vector3 (const vector2<assign_t>& init, assign_t init_z)
        : x (init.x), y (init.y), z (init_z)
    { }

    static const self origin() { return self(0, 0, 0); }
    static const self zero()   { return self(0, 0, 0); }

    value_type& operator[] (size_t index)
    {
        assert (index < 3);
        return (&x)[index];
    }

    const value_type operator[] (size_t index) const
    {
        assert (index < 3);
        return (&x)[index];
    }

    bool operator< (const self& compare) const
    {
        if (z < compare.z)
            return true;
        else if (z == compare.z)
        {
            if (y < compare.y)
                return true;
            else if (y == compare.y)
                return x < compare.x;
        }

        return false;
    }

    template <typename other>
    bool operator== (const vector3<other>& compare) const
    {
        return x == compare.x && y == compare.y && z == compare.z;
    }

    template <typename other>
    bool operator!= (const vector3<other>& compare) const
    {
        return !operator==(compare);
    }

    bool operator> (const self& compare) const
    {
        if (z > compare.z)
            return true;
        else if (z == compare.z)
        {
            if (y > compare.y)
                return true;
            else if (y == compare.y)
                return x > compare.x;
        }

        return false;
    }

public:
    self& operator+= (const vector2<type>& add)
    {
        x += add.x; y += add.y;
        return *this;
    }

    self& operator+= (const vector3<type>& add)
    {
        x += add.x; y += add.y; z += add.z;
        return *this;
    }

    self& operator-= (const vector2<type>& sub)
    {
        x -= sub.x; y -= sub.y;
        return *this;
    }

    self& operator-= (const vector3<type>& sub)
    {
        x -= sub.x; y -= sub.y; z -= sub.z;
        return *this;
    }

    self& operator*= (const self& mul)
    {
        x *= mul.x; y *= mul.y; z *= mul.z;
        return *this;
    }

    self& operator*= (value_type mul)
    {
        x *= mul; y *= mul; z *= mul;
        return *this;
    }

    self& operator/= (const self& div)
    {
        x /= div.x; y /= div.y; z /= div.z;
        return *this;
    }

    self& operator/= (value_type div)
    {
        x /= div; y /= div; z /= div;
        return *this;
    }

    self& operator%= (int mod)
    {
        x %= mod; y %= mod; z %= mod;
        return *this;
    }

    self& operator++ ()
    {
        ++x; ++y; ++z;
        return *this;
    }

    self& operator-- ()
    {
        --x; --y; --z;
        return *this;
    }

    const self operator- () const
    {
        return self(-x, -y, -z);
    }
};


//---------------------------------------------------------------------------

template <typename t, typename t2>
const vector3<t> operator+ (vector3<t> lhs, const vector3<t2>& rhs)
{
    return lhs += rhs;
}

template <typename t>
const vector3<t> operator+ (vector3<t> lhs, const vector2<t>& rhs)
{
    lhs.x += rhs.x; lhs.y += rhs.y;
    return lhs;
}

template <typename t>
const vector3<t> operator+ (const vector2<t>& lhs, vector3<t> rhs)
{
    rhs.x += lhs.x; rhs.y += lhs.y;
    return rhs;
}

template <typename t, typename t2>
const vector3<t> operator- (vector3<t> lhs, const vector3<t2>& rhs)
{
    return lhs -= rhs;
}

template <typename t>
const vector3<t> operator- (vector3<t> lhs, const vector2<t>& rhs)
{
    lhs.x -= rhs.x; lhs.y -= rhs.y;
    return lhs;
}

template <typename t>
const vector3<t> operator* (vector3<t> p, const vector3<t>& mul)
{
    return p *= mul;
}

template <typename t, typename mul_t>
const vector3<t> operator* (vector3<t> p, mul_t s)
{
    return p *= s;
}

template <typename t>
const vector3<t> operator/ (vector3<t> p, const vector3<t>& div)
{
    return p /= div;
}

template <typename t, typename div_t>
const vector3<t> operator/ (vector3<t> p, div_t div)
{
    return p /= div;
}

template <typename t>
const vector3<t> operator% (vector3<t> p, int mod)
{
    return p %= mod;
}

//---------------------------------------------------------------------------

/** Calculate the Manhattan length of a vector.
 * This is a specialized version of the one in algorithm.hpp, because
 * the call to std::abs is not needed for unsigned ints. */
template <> inline
unsigned int manhattan_length<vector3<unsigned int>>(const vector3<unsigned int>& v)
{
    return v[0] + v[1] + v[2];
}

//---------------------------------------------------------------------------

/** Raise the elements of a vector to a power. */
template <typename t>
const vector3<t> pow(const vector3<t>& v, t p)
{
    return vector3<t>(std::pow(v.x, p), std::pow(v.y, p), std::pow(v.z, p));
}

/** Clamp all elements between given limits. */
template <typename t>
const vector3<t> clamp_elements(const vector3<t>& v, t l, t h)
{
    return vector3<t>(clamp(v.x, l, h), clamp(v.y, l, h), clamp(v.z, l, h));
}

/** Calculate the cross product of two vectors. */
template <typename t>
const vector3<t> cross_prod(const vector3<t>& l, const vector3<t>& r)
{
    return vector3<t>(l.y*r.z - l.z*r.y,
                         l.z*r.x - l.x*r.z,
                         l.x*r.y - l.y*r.x);
}

/** Convert spherical coordinates to cartesian coordinates. */
template <typename t>
const vector3<t> from_spherical(t phi, t theta)
{
    auto sin_th (std::sin(theta));
    return vector3<t>(sin_th * std::sin(phi), sin_th * std::cos(phi),
                      std::cos(theta));
}

/** Convert spherical coordinates to cartesian coordinates. */
template <typename t>
const vector3<t> from_spherical(t phi, t theta, t distance)
{
    return from_spherical(phi, theta) * distance;
}

/** Convert spherical coordinates to cartesian coordinates. */
template <typename t>
const vector3<t> from_spherical(const vector2<t>& yaw_pitch)
{
    return from_spherical(yaw_pitch.x, yaw_pitch.y);
}

/** Convert spherical coordinates to cartesian coordinates. */
template <typename t>
const vector3<t> from_spherical(const vector3<t>& sph)
{
    return from_spherical(sph.x, sph.y, sph.z);
}

/** Convert cartesian coordinates to spherical coordinates. */
template <typename t>
const vector3<t> to_spherical(const vector3<t>& c)
{
    t radius (length(c));
    return vector3<t>(std::atan2(c.x, c.y), std::acos(c.z / radius), radius);
}

/** Round a coordinate to the nearest integer. */
template <typename t>
const vector3<int> round(const vector3<t>& in)
{
    return vector3<int>(::round(in.x), ::round(in.y), ::round(in.z));
}

/** Round a coordinate towards zero. */
template <typename t>
const vector3<int> round0(const vector3<t>& in)
{
    return vector3<int>(round_to_zero(in.x), round_to_zero(in.y), round_to_zero(in.z));
}

/** Round a coordinate down. */
template <typename t>
const vector3<int> floor(const vector3<t>& in)
{
    return vector3<int>(::floor(in.x), ::floor(in.y), ::floor(in.z));
}

/** Round a coordinate up. */
template <typename t>
const vector3<int> ceil(const vector3<t>& in)
{
    return vector3<int>(::ceil(in.x), ::ceil(in.y), ::ceil(in.z));
}

/** Return the sign of x, y, and z. \sa sign() */
template <typename t>
const vector3<int> signs(const vector3<t>& in)
{
    return vector3<int>(sign(in.x), sign(in.y), sign(in.z));
}

/** Return the absolute values of x, y, and z. */
template <typename t>
const vector3<t> absolute(const vector3<t>& in)
{
    return vector3<t>(std::abs(in.x), std::abs(in.y), std::abs(in.z));
}

/** Return the difference between two vectors.
 * \return  The difference between a and b  (always positive) */
template <typename t>
const vector3<t> diff (const vector3<t>& a, const vector3<t>& b)
{
    return vector3<t>(diff(a.x, b.x), diff(a.y, b.y), diff(a.z, b.z));
}

/** Flatten a vector3 to a vector2. */
template <typename t>
vector2<t> flat (const vector3<t>& a)
{
    return vector2<t>(a.x, a.y);
}

} // namespace hexa

//---------------------------------------------------------------------------

namespace std {

/** Hash function, required for std::unordered_map. */
template <typename t>
struct hash <hexa::vector3<t>>
    : public std::unary_function<hexa::vector3<t>, size_t>
{
    size_t operator() (const hexa::vector3<t>& v) const
        { return v.x ^ (v.y << 10) ^ (v.y >> 22) ^ (v.z << 20) ^ (v.z >> 12); }
};


/** Print a vector3 to a stream. */
template <typename t> inline
ostream& operator<< (ostream& str, const hexa::vector3<t>& vtx)
{
    return str << '(' << vtx.x << ' ' << vtx.y << ' ' << vtx.z << ')';
}

template <> inline
ostream& operator<< (ostream& str, const hexa::vector3<int8_t>& vtx)
{
    return str << '(' << (int)vtx.x << ' ' << (int)vtx.y << ' ' << (int)vtx.z << ')';
}

template <> inline
ostream& operator<< (ostream& str, const hexa::vector3<uint8_t>& vtx)
{
    return str << '(' << (int)vtx.x << ' ' << (int)vtx.y << ' ' << (int)vtx.z << ')';
}

/** Pull a vector3 from a stream. */
template <typename t> inline
istream& operator>> (istream& str, hexa::vector3<t>& vtx)
{
    return str >> vtx.x >> vtx.y >> vtx.z;
}

template <typename t> inline
string to_string (const hexa::vector3<t>& vtx)
{
    std::stringstream s;
    s << vtx;
    return s.str();
}

} // namespace std

