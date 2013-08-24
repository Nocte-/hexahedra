//---------------------------------------------------------------------------
/// \file   hexa/vector4.hpp
/// \brief  4-D vector class
//
// This file is part of Hexahedra.
//
// Hexahedra is free software; you can redistribute it and/or modify it
// under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation; either version 4 of the License, or
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
#include <unordered_map>
#include "algorithm.hpp"
#include "vector3.hpp"

namespace hexa {

/** A 4-D vector. */
template <typename type>
class vector4
{
    typedef vector4<type>    self;

public:
    type    x;
    type    y;
    type    z;
    type    w;

    typedef type        value_type;

public:
    vector4 () {}

    vector4 (type init_x, type init_y, type init_z, type init_w)
        : x (init_x), y (init_y), z (init_z), w (init_w)
    { }

    explicit vector4 (type scalar)
        : x (scalar), y (scalar), z (scalar), w (scalar)
    { }

    explicit vector4 (const type* ptr)
        : x (ptr[0]), y (ptr[1]), z (ptr[2]), w (ptr[3])
    { }

    template <typename assign_t>
    vector4 (const vector4<assign_t>& init)
        : x (init.x), y (init.y), z (init.z), w (init.w)
    { }

    template <typename assign_t>
    vector4 (const vector3<assign_t>& init, assign_t init_w = 1)
        : x (init.x), y (init.y), z (init.z), w (init_w)
    { }

    static const self zero()   { return self(0, 0, 0, 0); }
    static const self origin() { return self(0, 0, 0, 0); }

    value_type& operator[] (size_t index)
    {
        assert (index < 4);
        return (&x)[index];
    }

    const value_type operator[] (size_t index) const
    {
        assert (index < 4);
        return (&x)[index];
    }

    bool operator< (const self& compare) const
    {
        if (x < compare.x)
            return true;
        else if (x == compare.x)
        {
            if (y < compare.y)
                return true;
            else if (y == compare.y)
            {
                if (z < compare.z)
                    return true;
                else if (z == compare.z)
                    return w < compare.w;
            }
        }

        return false;
    }

    template <typename other>
    bool operator== (const vector4<other>& compare) const
    {
        return x == compare.x && y == compare.y && z == compare.z && w == compare.w;
    }

    template <typename other>
    bool operator!= (const vector4<other>& compare) const
    {
        return !operator==(compare);
    }

    bool operator> (const self& compare) const
    {
        if (x > compare.x)
            return true;
        else if (x == compare.x)
        {
            if (y > compare.y)
                return true;
            else if (y == compare.y)
            {
                if (z > compare.z)
                    return true;
                else if (z == compare.z)
                    return w > compare.w;
            }
        }

        return false;
    }

public:
    self& operator+= (const vector4<type>& add)
    {
        x += add.x; y += add.y; z += add.z; w += add.w;
        return *this;
    }

    self& operator-= (const vector4<type>& sub)
    {
        x -= sub.x; y -= sub.y; z -= sub.z; w -= sub.w;
        return *this;
    }

    self& operator*= (const self& mul)
    {
        x *= mul.x; y *= mul.y; z *= mul.z; w *= mul.w;
        return *this;
    }

    template<typename mul_type>
    self& operator*= (mul_type mul)
    {
        x *= mul; y *= mul; z *= mul; w *= mul;
        return *this;
    }

    self& operator/= (const self& div)
    {
        x /= div.x; y /= div.y; z /= div.z; w /= div.w;
        return *this;
    }

    template<typename div_type>
    self& operator/= (div_type div)
    {
        x /= div; y /= div; z /= div; w /= div;
        return *this;
    }

    self& operator%= (int mod)
    {
        x %= mod; y %= mod; z %= mod; w %= mod;
        return *this;
    }

    const self operator- () const
    {
        return self(-x, -y, -z, -w);
    }
};


//---------------------------------------------------------------------------

template <typename t, typename t2>
const vector4<t> operator+ (vector4<t> lhs, const vector4<t2>& rhs)
{
    return lhs += rhs;
}

template <typename t, typename t2>
const vector4<t> operator- (vector4<t> lhs, const vector4<t2>& rhs)
{
    return lhs -= rhs;
}

template <typename t>
const vector4<t> operator* (vector4<t> p, const vector4<t>& mul)
{
    return p *= mul;
}

template <typename t, typename mul_t>
const vector4<t> operator* (vector4<t> p, mul_t s)
{
    return p *= s;
}

template <typename t>
const vector4<t> operator/ (vector4<t> p, const vector4<t>& div)
{
    return p /= div;
}

template <typename t, typename div_t>
const vector4<t> operator/ (vector4<t> p, div_t div)
{
    return p /= div;
}

template <typename t>
const vector4<t> operator% (vector4<t> p, int mod)
{
    return p %= mod;
}

//---------------------------------------------------------------------------

/** Calculate the dot product of two vectors. */
template <typename t>
const t dot_prod(const vector3<t>& l, const vector4<t>& r)
{
    return l.x * r.x + l.y * r.y + l.z * r.z + r.w;
}

/** Calculate the dot product of two vectors. */
template <typename t>
const t dot_prod(const vector4<t>& l, const vector3<t>& r)
{
    return l.x * r.x + l.y * r.y + l.z * r.z + l.w;
}

/** Round a coordinate to the nearest integer. */
template <typename t>
const vector4<int> round(const vector4<t>& in)
{
    return vector4<int>(::round(in.x), ::round(in.y), ::round(in.z), ::round(in.w));
}

/** Round a coordinate towards zero. */
template <typename t>
const vector4<int> round0(const vector4<t>& in)
{
    return vector4<int>(round_to_zero(in.x), round_to_zero(in.y), round_to_zero(in.z),
                        round_to_zero(in.w));
}

/** Round a coordinate down. */
template <typename t>
const vector4<int> floor(const vector4<t>& in)
{
    return vector4<int>(::floor(in.x), ::floor(in.y), ::floor(in.z), ::floor(in.w));
}

/** Round a coordinate up. */
template <typename t>
const vector4<int> ceil(const vector4<t>& in)
{
    return vector4<int>(::ceil(in.x), ::ceil(in.y), ::ceil(in.z), ::ceil(in.w));
}

template <typename t>
const vector4<int> signs(const vector4<t>& in)
{
    return vector4<int>(sign(in.x), sign(in.y), sign(in.z), sign(in.w));
}

template <typename t>
const vector4<t> absolute(const vector4<t>& in)
{
    return vector4<t>(std::abs(in.x), std::abs(in.y), std::abs(in.z), std::abs(in.w));
}

/** Return the difference between two vectors.
 * \return  The difference between a and b  (always positive) */
template <typename type>
const vector4<type> diff (vector4<type> a, vector4<type> b)
{
    return vector4<type>(diff(a.x, b.x), diff(a.y, b.y),
                         diff(a.z, b.z), diff(a.w, b.w));
}

/// \cond show_hidden

template <class type>
struct vector4_output_impl
{
    static std::ostream& print(std::ostream& str, const vector4<type>& vtx)
        { return str << '(' << vtx.x << ' ' << vtx.y << ' ' << vtx.z << ' ' << vtx.w << ')'; }
};

template <>
struct vector4_output_impl<int8_t>
{
    static std::ostream& print(std::ostream& str, const vector4<int8_t>& vtx)
    {
        return str << '(' << int(vtx.x) << ' ' << int(vtx.y) << ' '
                   << int(vtx.z) << ' ' << int(vtx.w) << ')';
    }
};

/// \endcond

} // namespace hexa


namespace std {

/** Hash function, required for std::unordered_map. */
template <class type>
struct hash <hexa::vector4<type>>
    : public std::unary_function<hexa::vector4<type>, size_t>
{
    size_t operator() (const hexa::vector4<type>& v) const
        { return v.x ^ (v.y << 10) ^ (v.y >> 22) ^ (v.z << 20) ^ (v.z >> 12) ^ v.w; }
};


/** Print a vector4 to a stream. */
template <class type>
ostream& operator<< (ostream& str, const hexa::vector4<type>& vtx)
{
    return hexa::vector4_output_impl<type>::print(str, vtx);
}

} // namespace std
