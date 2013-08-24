//---------------------------------------------------------------------------
/// \file   color.hpp
/// \brief  A color with floating-point red, green, blue, and alpha channels
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
// Copyright (C) 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <iostream>
#include "algorithm.hpp"
#include "vector3.hpp"

namespace hexa {

/** Floating point precision red/green/blue color. */
class color : public vector3<float>
{
    typedef vector3<float>  base;

public:
    using base::operator[];
    using base::operator*=;
    using base::operator/=;

public:
    color () { }

    color (base init) : base(init) { }

    color (value_type red, value_type green, value_type blue)
        : base (red, green, blue)
    {
    }

    // Dirty, but very convenient to interoperate with OpenGL:

    float*       as_ptr()
        { return reinterpret_cast<float*>(this); }

    const float* as_ptr() const
        { return reinterpret_cast<const float*>(this); }

    const value_type  r() const { return (*this)[0]; }
    const value_type  g() const { return (*this)[1]; }
    const value_type  b() const { return (*this)[2]; }

    value_type& r() { return (*this)[0]; }
    value_type& g() { return (*this)[1]; }
    value_type& b() { return (*this)[2]; }

    // Alternative names for CIE colors

    const value_type  x() const { return (*this)[0]; }
    const value_type  y() const { return (*this)[1]; }
    const value_type  Y() const { return (*this)[2]; }

    value_type& x() { return (*this)[0]; }
    value_type& y() { return (*this)[1]; }
    value_type& Y() { return (*this)[2]; }

    static color black()    { return color(0, 0, 0); }
    static color white()    { return color(1, 1, 1); }
    static color red()      { return color(1, 0, 0); }
    static color green()    { return color(0, 1, 0); }
    static color blue()     { return color(0, 0, 1); }
    static color yellow()   { return color(1, 1, 0); }
    static color magenta()  { return color(1, 0, 1); }
    static color cyan()     { return color(0, 1, 1); }
};


inline
const color operator* (color lhs, color rhs)
    { return lhs *= rhs; }

inline
const color operator* (color lhs, color::value_type rhs)
    { return lhs *= rhs; }

inline
const color operator* (color::value_type lhs, color rhs)
    { return rhs *= lhs; }

//---------------------------------------------------------------------------

/** Color with transparency channel. */
class color_alpha : public color
{
public:
    color_alpha () { }

    color_alpha (value_type red, value_type green, value_type blue,
                 value_type alpha = 1)
        : color (red, green, blue)
        , a_ (alpha)
    {
    }

    color_alpha (color init, value_type alpha = 1)
        : color (init)
        , a_ (alpha)
    {
    }

    const value_type  a() const   { return a_; }
    value_type&       a()         { return a_; }

    color_alpha& operator*= (const color& m)
    {
        color::operator*=(m);
        return *this;
    }

    color_alpha& operator*= (const color_alpha& m)
    {
        color::operator*=(m);
        a_ *= m.a_;
        return *this;
    }

    color_alpha& operator*= (value_type m)
    {
        color::operator*=(m);
        a_ *= m;
        return *this;
    }

protected:
    value_type a_;
};


inline
const color_alpha operator* (color_alpha lhs, const color_alpha& rhs)
    { return lhs *= rhs; }

inline
const color_alpha operator* (color_alpha lhs, color_alpha::value_type rhs)
    { return lhs *= rhs; }

inline
const color_alpha operator* (color_alpha::value_type lhs, color_alpha rhs)
    { return rhs *= lhs; }

//---------------------------------------------------------------------------

inline const color xyY_to_srgb(const color& in)
{
    // First convert xyY to XYZ tristimulus values
    color t (in.Y() * in.x() / in.y(), in.Y(), in.Y() * (1.0f - in.x() - in.y()) / in.y());

    // Then perform the matrix multiplication to linear RGB
    color lin( 3.2406f * t[0] + -1.5372f * t[1] + -0.4986f * t[2],
              -0.9689f * t[0] +  1.8758f * t[1] +  0.0415f * t[2],
               0.0557f * t[0] + -0.2040f * t[1] +  1.0570f * t[2]);

    // Assume a 80 cd/m2 display.
    lin /= 80.f;

    // Finally, apply gamma and return sRGB
    const float gamma (1.f / 2.2f);
    return pow(clamp_elements(lin, 0.f, 1.f), gamma);
}

} // namespace hexa

namespace std
{

inline
ostream& operator<< (ostream& str, const hexa::color& col)
{
    return str << '(' << col.r() << ';' << col.g() << ';' << col.b() << ')';
}

inline
ostream& operator<< (ostream& str, const hexa::color_alpha& col)
{
    return str << '(' << col.r() << ';' << col.g() << ';' << col.b() << ';'
               << col.a() << ')';
}
} // namespace std

