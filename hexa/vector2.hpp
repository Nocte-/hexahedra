//---------------------------------------------------------------------------
/// \file   hexa/vector2.hpp
/// \brief  2-D vector class
//
// This file is part of Hexahedra.
//
// Hexahedra is free software; you can redistribute it and/or modify it
// under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation; either version 2 of the License, or
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

namespace hexa {

// Forward declaration
template <typename type>
class vector3;

/** A 2-D vector. */
template <typename type>
class vector2
{
    typedef vector2<type>    self;

public:
    /** The x ordinate (increases east) */
    type    x;
    /** The y ordinate (increases north) */
    type    y;

    typedef type        value_type;

public:
    vector2 () {}

    vector2 (type init_x, type init_y)
        : x (init_x), y (init_y)
    { }

    vector2 (std::initializer_list<type> l)
    {
        assert(l.size() == 2);
        auto i (std::begin(l));
        x = *i++;
        y = *i;
    }

    explicit vector2 (type scalar)
        : x (scalar), y (scalar)
    { }

    explicit vector2 (const type* ptr)
        : x (ptr[0]), y (ptr[1])
    { }

    vector2 (const vector3<type>& flatten)
        : x (flatten.x), y (flatten.y)
    { }

    template <typename conv_t>
    vector2 (vector2<conv_t> conv)
        : x (static_cast<type>(conv.x)), y (static_cast<type>(conv.y))
    { }

    static self origin() { return self(0); }

    value_type& operator[] (size_t index)
    {
        assert (index < 2);
        return (&x)[index];
    }

    value_type operator[] (size_t index) const
    {
        assert (index < 2);
        return (&x)[index];
    }

    bool operator< (const self& compare) const
    {
        if (x < compare.x)
            return true;
        else if (x == compare.x)
            return y < compare.y;

        return false;
    }

    template <typename other>
    bool operator== (const vector2<other>& compare) const
    {
        return x == compare.x && y == compare.y;
    }

    template <typename other>
    bool operator!= (const vector2<other>& compare) const
    {
        return !operator==(compare);
    }

    bool operator> (const self& compare) const
    {
        if (x > compare.x)
            return true;
        else if (x == compare.x)
            return y > compare.y;

        return false;
    }

public:
    self& operator+= (const vector2<type>& add)
    {
        x += add.x; y += add.y;
        return *this;
    }

    self& operator-= (const vector2<type>& sub)
    {
        x -= sub.x; y -= sub.y;
        return *this;
    }

    self& operator*= (const self& mul)
    {
        x *= mul.x; y *= mul.y;
        return *this;
    }

    template<typename mul_type>
    self& operator*= (mul_type mul)
    {
        x *= mul; y *= mul;
        return *this;
    }

    self& operator/= (const self& div)
    {
        x /= div.x; y /= div.y;
        return *this;
    }

    template<typename div_type>
    self& operator/= (div_type div)
    {
        x /= div; y /= div;
        return *this;
    }

    /** Modulo operator, only works for power-of-two modulos.
     *  The resulting values are always positive. */
    self& operator%= (int mod)
    {
        --mod;
        x &= mod; y &= mod;
        return *this;
    }

    /** Modulo function, always returns a positive result. */
    self& mod (int mod)
    {
        x = (x%mod+mod)%mod;
        y = (y%mod+mod)%mod;
        return *this;
    }

    const self operator- () const
    {
        return self(-x, -y);
    }
};


//---------------------------------------------------------------------------

template <typename t, typename t2>
const vector2<t> operator+ (vector2<t> lhs, vector2<t2> rhs)
{
    return lhs += rhs;
}

template <typename t, typename t2>
const vector2<t> operator- (vector2<t> lhs, vector2<t2> rhs)
{
    return lhs -= rhs;
}

template <typename t>
const vector2<t> operator* (vector2<t> p, vector2<t> mul)
{
    return p *= mul;
}

template <typename t, typename mul_t>
const vector2<t> operator* (vector2<t> p, mul_t s)
{
    return p *= s;
}

template <typename t>
const vector2<t> operator/ (vector2<t> p, vector2<t> div)
{
    return p /= div;
}

template <typename t, typename div_t>
const vector2<t> operator/ (vector2<t> p, div_t div)
{
    return p /= div;
}

template <typename div_t>
const vector2<int8_t> operator/ (vector2<int8_t> p, div_t div)
{
    return {divd(p.x, div), divd(p.y, div)};
}

template <typename div_t>
const vector2<int16_t> operator/ (vector2<int16_t> p, div_t div)
{
    return {divd(p.x, div), divd(p.y, div)};
}

template <typename div_t>
const vector2<int32_t> operator/ (vector2<int32_t> p, div_t div)
{
    return {divd(p.x, div), divd(p.y, div)};
}

template <typename t>
const vector2<t> operator>> (vector2<t> p, int sh)
{
    return {p.x >> sh, p.y >> sh};
}

template <typename t>
const vector2<t> operator% (vector2<t> p, int mod)
{
    return p %= mod;
}

//---------------------------------------------------------------------------

/** Convert polar coordinates to cartesian coordinates. */
template <typename t>
const vector2<t> from_polar(t theta)
{
    return vector2<t>(std::sin(theta), std::cos(theta));
}

/** Convert polar coordinates to cartesian coordinates. */
template <typename t>
const vector2<t> from_polar(t theta, t r)
{
    return from_polar(theta) * r;
}

/** Round a coordinate to the nearest integer. */
template <typename t>
const vector2<int> round(const vector2<t>& in)
{
    return vector2<int>(::round(in.x), ::round(in.y), ::round(in.z));
}

/** Round a coordinate down. */
template <typename t>
const vector2<int> floor(const vector2<t>& in)
{
    return vector2<int>(::floor(in.x), ::floor(in.y), ::floor(in.z));
}

/** Return the difference between two vectors.
 * \return  The difference between a and b  (always positive) */
template <typename type>
const vector2<type> diff (vector2<type> a, vector2<type> b)
{
    return vector2<type>(diff(a.x, b.x), diff(a.y, b.y));
}

} // namespace hexa

//---------------------------------------------------------------------------

namespace std {

/** Hash function, required for std::unordered_map. */
template <typename type>
struct hash <hexa::vector2<type>>
    : public std::unary_function<hexa::vector2<type>, size_t>
{
    size_t operator() (const hexa::vector2<type>& v) const
        { return v.x ^ (v.y << 9) ^ (v.y >> 21); }
};

template <>
struct hash <hexa::vector2<uint8_t>>
    : public std::unary_function<hexa::vector2<uint8_t>, size_t>
{
    size_t operator() (const hexa::vector2<uint8_t>& v) const
    {
        return v.x + (uint16_t(v.y) << 8);
    }
};


/** Print a vector2 to a stream. */
template <typename type> inline
ostream& operator<< (ostream& str, const hexa::vector2<type>& vtx)
{
    return str << '(' << vtx.x << ' ' << vtx.y << ')';
}

template <> inline
ostream& operator<< (ostream& str, const hexa::vector2<int8_t>& vtx)
{
    return str << '(' << (int)vtx.x << ' ' << (int)vtx.y << ')';
}

template <> inline
ostream& operator<< (ostream& str, const hexa::vector2<uint8_t>& vtx)
{
    return str << '(' << (int)vtx.x << ' ' << (int)vtx.y << ')';
}

template <typename t> inline
string to_string (const hexa::vector2<t>& vtx)
{
    std::stringstream s;
    s << vtx;
    return s.str();
}

} // namespace std

