//---------------------------------------------------------------------------
/// \file   wfpos.hpp
/// \brief  Floating point vector class that tries to avoid rounding errors
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
// Copyright 2013, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <unordered_map>

#include "algorithm.hpp"
#include "basic_types.hpp"

namespace hexa
{

/** World-scale floating point vector. */
class wfvec
{
public:
    world_rel_coordinates pos;
    vector frac;

public:
    wfvec() {}

    wfvec(int32_t x, int32_t y, int32_t z)
        : pos(x, y, z)
        , frac(0, 0, 0)
    {
    }

    wfvec(float x, float y, float z)
        : pos(0, 0, 0)
        , frac(x, y, z)
    {
        normalize();
    }

    wfvec(world_rel_coordinates a, vector b)
        : pos(a)
        , frac(b)
    {
    }

    void normalize()
    {
        float i;
        frac.x = std::modf(frac.x, &i);
        pos.x += static_cast<int>(i);
        frac.y = std::modf(frac.y, &i);
        pos.y += static_cast<int>(i);
        frac.z = std::modf(frac.z, &i);
        pos.z += static_cast<int>(i);
    }

    vector float_vec() const
    {
        return vector(frac.x + pos.x, frac.y + pos.y, frac.z + pos.z);
    }

    vector3<double> double_vec() const
    {
        return vector3<double>((double)frac.x + pos.x, (double)frac.y + pos.y,
                               (double)frac.z + pos.z);
    }

    bool operator==(const wfvec& compare) const
    {
        return pos == compare.pos && frac == compare.frac;
    }

    bool operator!=(const wfvec& compare) const
    {
        return !operator==(compare);
    }

public:
    wfvec& operator+=(const wfvec& add)
    {
        pos += add.pos;
        frac += add.frac;
        return *this;
    }

    wfvec& operator+=(const vector& add)
    {
        frac += add;
        return *this;
    }

    wfvec& operator-=(const wfvec& sub)
    {
        pos -= sub.pos;
        frac -= sub.frac;
        return *this;
    }

    wfvec& operator-=(const vector& sub)
    {
        frac -= sub;
        return *this;
    }

    wfvec& operator*=(float v)
    {
        pos *= v;
        frac *= v;
        return *this;
    }

    wfvec& operator/=(float v)
    {
        pos /= v;
        frac /= v;
        return *this;
    }

    const wfvec operator-() const { return wfvec(-pos, -frac); }
};

//---------------------------------------------------------------------------

/** World-scale floating point position. */
class wfpos
{
public:
    world_coordinates pos;
    vector frac;

public:
    wfpos() {}

    wfpos(std::initializer_list<uint32_t> l)
        : pos(l)
        , frac(0, 0, 0)
    {
    }

    wfpos(uint32_t x, uint32_t y, uint32_t z)
        : pos(x, y, z)
        , frac(0, 0, 0)
    {
    }

    wfpos(float x, float y, float z)
        : pos(0, 0, 0)
        , frac(x, y, z)
    {
        normalize();
    }

    wfpos(world_coordinates a, vector b)
        : pos(a)
        , frac(b)
    {
    }

    void normalize()
    {
        float i;
        frac.x = std::modf(frac.x, &i);
        pos.x += static_cast<int>(i);
        frac.y = std::modf(frac.y, &i);
        pos.y += static_cast<int>(i);
        frac.z = std::modf(frac.z, &i);
        pos.z += static_cast<int>(i);
    }

    wfpos normalized() const
    {
        wfpos result(*this);
        result.normalize();
        return result;
    }

    world_coordinates int_pos() const
    {
        return world_coordinates(frac.x + pos.x, frac.y + pos.y,
                                 frac.z + pos.z);
    }

    vector float_pos() const
    {
        return vector(frac.x + pos.x, frac.y + pos.y, frac.z + pos.z);
    }

    vector3<double> double_pos() const
    {
        return vector3<double>((double)frac.x + pos.x, (double)frac.y + pos.y,
                               (double)frac.z + pos.z);
    }

    vector relative_to(const world_coordinates& rel) const
    {
        return vector(frac.x + int32_t(pos.x - rel.x),
                      frac.y + int32_t(pos.y - rel.y),
                      frac.z + int32_t(pos.z - rel.z));
    }
    vector relative_to(const wfpos& rel) const
    {
        return vector(frac.x - rel.frac.x + int32_t(pos.x - rel.pos.x),
                      frac.y - rel.frac.y + int32_t(pos.y - rel.pos.y),
                      frac.z - rel.frac.z + int32_t(pos.z - rel.pos.z));
    }

    static const wfpos origin() { return wfpos(0u, 0u, 0u); }
    static const wfpos zero() { return wfpos(0u, 0u, 0u); }

    bool operator==(const wfpos& compare) const
    {
        return pos == compare.pos && frac == compare.frac;
    }

    bool operator!=(const wfpos& compare) const
    {
        return !operator==(compare);
    }

    bool operator<(const wfpos& compare) const
    {
        return pos < compare.pos
               || (pos == compare.pos && frac < compare.frac);
    }

    bool operator>(const wfpos& compare) const
    {
        return pos > compare.pos
               || (pos == compare.pos && frac > compare.frac);
    }

public:
    wfpos& operator+=(const wfvec& add)
    {
        pos += add.pos;
        frac += add.frac;
        return *this;
    }

    wfpos& operator+=(const vector& add)
    {
        frac += add;
        return *this;
    }

    wfpos& operator+=(const world_vector& add)
    {
        pos += add;
        return *this;
    }

    wfpos& operator-=(const wfvec& sub)
    {
        pos -= sub.pos;
        frac -= sub.frac;
        return *this;
    }

    wfpos& operator-=(const vector& sub)
    {
        frac -= sub;
        return *this;
    }

    wfpos& operator-=(const world_vector& sub)
    {
        pos -= sub;
        return *this;
    }
};

//---------------------------------------------------------------------------

inline wfvec operator-(const wfpos& lhs, const wfpos& rhs)
{
    return wfvec(lhs.pos - rhs.pos, lhs.frac - rhs.frac);
}

inline wfvec operator-(const wfpos& lhs, const world_coordinates& rhs)
{
    return wfvec(lhs.pos - rhs, lhs.frac);
}

inline wfvec operator-(const world_coordinates& lhs, const wfpos& rhs)
{
    return wfvec(lhs - rhs.pos, -rhs.frac);
}

inline wfvec operator+(wfvec lhs, const wfvec& rhs)
{
    return lhs += rhs;
}

inline wfvec operator+(wfvec lhs, const vector& rhs)
{
    return lhs += rhs;
}

inline wfvec operator-(wfvec lhs, const wfvec& rhs)
{
    return lhs -= rhs;
}

inline wfvec operator-(wfvec lhs, const vector& rhs)
{
    return lhs -= rhs;
}

//---------------------------------------------------------------------------

inline wfpos operator+(wfpos lhs, const wfvec& rhs)
{
    return lhs += rhs;
}

inline wfpos operator+(wfpos lhs, const world_vector& rhs)
{
    return lhs += rhs;
}

inline wfpos operator+(wfpos lhs, const hexa::vector& rhs)
{
    return lhs += rhs;
}

inline wfpos operator-(wfpos lhs, const wfvec& rhs)
{
    return lhs -= rhs;
}

inline wfpos operator-(wfpos lhs, const world_vector& rhs)
{
    return lhs -= rhs;
}

inline wfpos operator-(wfpos lhs, const hexa::vector& rhs)
{
    return lhs -= rhs;
}

inline wfvec operator*(wfvec lhs, float rhs)
{
    return lhs *= rhs;
}

inline wfvec operator*(float lhs, wfvec rhs)
{
    return rhs *= lhs;
}

inline wfvec operator/(wfvec lhs, float rhs)
{
    return lhs /= rhs;
}

// Some specializations of functions found in algorithms.hpp:

template <>
inline const double squared_length(const wfvec& v)
{
    return squared_length(v.double_vec());
}

template <>
inline const double squared_distance(const wfpos& a, const wfpos& b)
{
    return squared_length(a.relative_to(b));
}

template <>
inline const wfpos lerp(const wfpos& from, const wfpos& to, double amount)
{
    return from + to.relative_to(from) * amount;
}

} // namespace hexa

//---------------------------------------------------------------------------

namespace std
{

/** Print a wfpos to a stream. */
inline ostream& operator<<(ostream& str, const hexa::wfpos& vtx)
{
    return str << '[' << vtx.pos << ':' << vtx.frac << ']';
}

inline string to_string(const hexa::wfpos& vtx)
{
    std::stringstream s;
    s << vtx;
    return s.str();
}

} // namespace std
