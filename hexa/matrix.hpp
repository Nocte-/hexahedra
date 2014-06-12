//---------------------------------------------------------------------------
/// \file   matrix.hpp
/// \brief  Matrix classes
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
// Copyright 2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include <boost/numeric/ublas/matrix.hpp>
#include "basic_types.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

namespace hexa
{

/** OpenGL-compatible (column-major ordered) 3x3 matrix. */
template <typename t>
class matrix3
{
    typedef std::array<t, 3 * 3> storage;

public:
    typedef t value_type;
    typedef matrix3<t> self;

    typedef typename storage::iterator iterator;
    typedef typename storage::const_iterator const_iterator;

public:
    matrix3() {}

    matrix3(t x1, t y1, t z1, t x2, t y2, t z2, t x3, t y3, t z3)
    {
        m_[0] = x1;
        m_[1] = x2;
        m_[2] = x3;
        m_[3] = y1;
        m_[4] = y2;
        m_[5] = y3;
        m_[6] = z1;
        m_[7] = z2;
        m_[8] = z3;
    }

    value_type& operator()(size_t i, size_t j)
    {
        assert(i < 3);
        assert(j < 3);
        return m_[i + j * 3];
    }

    value_type operator()(size_t i, size_t j) const
    {
        assert(i < 3);
        assert(j < 3);
        return m_[i + j * 3];
    }

    self& operator*=(value_type mul)
    {
        for (value_type& i : m_)
            i *= mul;
        return *this;
    }

    self& operator/=(value_type div)
    {
        for (value_type& i : m_)
            i /= div;
        return *this;
    }

    const value_type* as_ptr() const { return &m_[0]; }
    value_type* as_ptr() { return &m_[0]; }

public:
    size_t size() const { return m_.size(); }
    iterator begin() { return m_.begin(); }
    iterator end() { return m_.end(); }
    const_iterator begin() const { return m_.begin(); }
    const_iterator end() const { return m_.end(); }

    value_type& operator[](size_t i)
    {
        assert(i < size());
        return m_[i];
    }

    const value_type operator[](size_t i) const
    {
        assert(i < size());
        return m_[i];
    }

public:
    static matrix3<t> zero() { return matrix3<t>(0, 0, 0, 0, 0, 0, 0, 0, 0); }

    static matrix3<t> identity()
    {
        return matrix3<t>(1, 0, 0, 0, 1, 0, 0, 0, 1);
    }

    static matrix3<t> scale2(t mul)
    {
        return matrix3<t>(mul, 0, 0, 0, mul, 0, 0, 0, mul);
    }

    static matrix3<t> diagonal(const vector3<t>& x)
    {
        return matrix3<t>(x.x, 0, 0, 0, x.y, 0, 0, 0, x.z);
    }

    static matrix3<t> cross(const vector3<t>& x)
    {
        return matrix3<t>(0, x.z, -x.y, -x.z, 0, x.x, x.y, x.x, 0);
    }

private:
    storage m_;
};

//---------------------------------------------------------------------------

template <typename T>
const matrix3<T> operator*(const matrix3<T>& l, const matrix3<T>& r)
{
    return {l(0, 0) * r(0, 0) + l(0, 1) * r(1, 0) + l(0, 2) * r(2, 0),
            l(0, 0) * r(0, 1) + l(0, 1) * r(1, 1) + l(0, 2) * r(2, 1),
            l(0, 0) * r(0, 2) + l(0, 1) * r(1, 2) + l(0, 2) * r(2, 2),

            l(1, 0) * r(0, 0) + l(1, 1) * r(1, 0) + l(1, 2) * r(2, 0),
            l(1, 0) * r(0, 1) + l(1, 1) * r(1, 1) + l(1, 2) * r(2, 1),
            l(1, 0) * r(0, 2) + l(1, 1) * r(1, 2) + l(1, 2) * r(2, 2),

            l(2, 0) * r(0, 0) + l(2, 1) * r(1, 0) + l(2, 2) * r(2, 0),
            l(2, 0) * r(0, 1) + l(2, 1) * r(1, 1) + l(2, 2) * r(2, 1),
            l(2, 0) * r(0, 2) + l(2, 1) * r(1, 2) + l(2, 2) * r(2, 2)};
}

template <typename T>
const vector3<T> operator*(const matrix3<T>& l, const vector3<T>& r)
{
    return {l(0, 0) * r[0] + l(0, 1) * r[1] + l(0, 2) * r[2],
            l(1, 0) * r[0] + l(1, 1) * r[1] + l(1, 2) * r[2],
            l(2, 0) * r[0] + l(2, 1) * r[1] + l(2, 2) * r[2]};
}

template <typename T>
const matrix3<T> operator*(matrix3<T> l, T r)
{
    return l *= r;
}

template <typename T>
matrix3<T> translate(matrix3<T> m, const vector2<T>& v)
{
    m(0, 2) += m(0, 0) * v.x + m(0, 1) * v.y;
    m(1, 2) += m(1, 0) * v.x + m(1, 1) * v.y;

    return m;
}

//---------------------------------------------------------------------------

/** OpenGL-compatible (column-major ordered) 3x4 matrix. */
template <typename t>
class matrix3x4
{
    typedef std::array<t, 3 * 4> storage;

public:
    typedef t value_type;
    typedef matrix3x4<t> self;

    typedef typename storage::iterator iterator;
    typedef typename storage::const_iterator const_iterator;

public:
    matrix3x4() {}

    matrix3x4(t x1, t y1, t z1, t w1, t x2, t y2, t z2, t w2, t x3, t y3, t z3,
              t w3)
    {
        m_[0] = x1;
        m_[1] = x2;
        m_[2] = x3;
        m_[3] = y1;
        m_[4] = y2;
        m_[5] = y3;
        m_[6] = z1;
        m_[7] = z2;
        m_[8] = z3;
        m_[9] = w1;
        m_[10] = w2;
        m_[11] = w3;
    }

    matrix3x4(const matrix3<t>& rotation, const vector3<t>& translation)
    {
        std::copy(rotation.begin(), rotation.end(), begin());
        m_[9] = translation.x;
        m_[10] = translation.y;
        m_[12] = translation.z;
    }

    value_type& operator()(size_t i, size_t j)
    {
        assert(i < 3);
        assert(j < 4);
        return m_[i + j * 3];
    }

    const value_type operator()(size_t i, size_t j) const
    {
        assert(i < 3);
        assert(j < 4);
        return m_[i + j * 3];
    }

    self& operator*=(value_type mul)
    {
        for (value_type& i : m_)
            i *= mul;
        return *this;
    }

    self& operator/=(value_type div)
    {
        for (value_type& i : m_)
            i /= div;
        return *this;
    }

    const value_type* as_ptr() const { return &m_[0]; }
    value_type* as_ptr() { return &m_[0]; }

public:
    size_t size() const { return m_.size(); }
    iterator begin() { return m_.begin(); }
    iterator end() { return m_.end(); }
    const_iterator begin() const { return m_.begin(); }
    const_iterator end() const { return m_.end(); }

    value_type& operator[](size_t i)
    {
        assert(i < size());
        return m_[i];
    }

    const value_type operator[](size_t i) const
    {
        assert(i < size());
        return m_[i];
    }

public:
    static matrix3x4<t> zero()
    {
        return matrix3x4<t>(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    static matrix3x4<t> identity()
    {
        return matrix3x4<t>(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0);
    }

    static matrix3x4<t> diagonal(const vector3<t>& x)
    {
        return matrix3x4<t>(x.x, 0, 0, 0, x.y, 0, 0, 0, x.z);
    }

    static matrix3x4<t> cross(const vector3<t>& x)
    {
        return matrix3x4<t>(0, x.z, -x.y, 0, -x.z, 0, x.x, 0, x.y, x.x, 0, 0);
    }

private:
    storage m_;
};

//---------------------------------------------------------------------------

template <typename t>
vector3<t> operator*(const matrix3x4<t>& l, const vector3<t>& r)
{
    return vector3<t>(
        l(0, 0) * r[0] + l(0, 1) * r[1] + l(0, 2) * r[2] + l(0, 3),
        l(1, 0) * r[0] + l(1, 1) * r[1] + l(1, 2) * r[2] + l(1, 3),
        l(2, 0) * r[0] + l(2, 1) * r[1] + l(2, 2) * r[2] + l(2, 3));
}

template <typename t>
matrix3x4<t> operator*(matrix3x4<t> l, t r)
{
    return l *= r;
}

//---------------------------------------------------------------------------

/** OpenGL-compatible (column-major ordered) 4x4 matrix. */
template <typename t>
class matrix4
{
    typedef std::array<t, 4 * 4> storage;

public:
    typedef t value_type;
    typedef matrix4<t> self;

    typedef typename storage::iterator iterator;
    typedef typename storage::const_iterator const_iterator;

public:
    matrix4() {}

    matrix4(const matrix3<value_type>& i)
    {
        auto p(i.as_ptr());
        m_[0] = p[0];
        m_[1] = p[1];
        m_[2] = p[2];
        m_[3] = 0;
        m_[4] = p[3];
        m_[5] = p[4];
        m_[6] = p[5];
        m_[7] = 0;
        m_[8] = p[6];
        m_[9] = p[7];
        m_[10] = p[8];
        m_[11] = 0;
        m_[12] = 0;
        m_[13] = 0;
        m_[14] = 0;
        m_[15] = 1;
    }

    matrix4(t x1, t y1, t z1, t w1, t x2, t y2, t z2, t w2, t x3, t y3, t z3,
            t w3, t x4, t y4, t z4, t w4)
    {
        m_[0] = x1;
        m_[1] = x2;
        m_[2] = x3;
        m_[3] = x4;
        m_[4] = y1;
        m_[5] = y2;
        m_[6] = y3;
        m_[7] = y4;
        m_[8] = z1;
        m_[9] = z2;
        m_[10] = z3;
        m_[11] = z4;
        m_[12] = w1;
        m_[13] = w2;
        m_[14] = w3;
        m_[15] = w4;
    }

    value_type& operator()(size_t i, size_t j)
    {
        assert(i < 4);
        assert(j < 4);
        return m_[i + j * 4];
    }

    const value_type operator()(size_t i, size_t j) const
    {
        assert(i < 4);
        assert(j < 4);
        return m_[i + j * 4];
    }

    self& operator*=(value_type mul)
    {
        for (value_type& i : m_)
            i *= mul;
        return *this;
    }

    self& operator/=(value_type div)
    {
        for (value_type& i : m_)
            i /= div;
        return *this;
    }
    
    bool operator== (const self& compare) const
    {
        return m_ == compare.m_;
    }

    bool operator!= (const self& compare) const
    {
        return m_ != compare.m_;
    }
    
    const value_type* as_ptr() const { return &m_[0]; }
    value_type* as_ptr() { return &m_[0]; }

public:
    size_t size() const { return m_.size(); }
    iterator begin() { return m_.begin(); }
    iterator end() { return m_.end(); }
    const_iterator begin() const { return m_.begin(); }
    const_iterator end() const { return m_.end(); }

    value_type& operator[](size_t i)
    {
        assert(i < size());
        return m_[i];
    }

    const value_type operator[](size_t i) const
    {
        assert(i < size());
        return m_[i];
    }

public:
    static matrix4<t> zero()
    {
        return matrix4<t>(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    }

    static matrix4<t> identity()
    {
        return matrix4<t>(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
    }

    static matrix4<t> scale(t mx, t my, t mz)
    {
        return matrix4<t>(mx, 0, 0, 0, 0, my, 0, 0, 0, 0, mz, 0, 0, 0, 0, 1);
    }

    static matrix4<t> flip()
    {
        return matrix4<t>(1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1);
    }

    static matrix4<t> scale3(t mul)
    {
        return matrix4<t>(mul, 0, 0, 0, 0, mul, 0, 0, 0, 0, mul, 0, 0, 0, 0,
                          1);
    }

    static matrix4<t> diagonal(const vector3<t>& x)
    {
        return matrix4<t>(x.x, 0, 0, 0, 0, x.y, 0, 0, 0, 0, x.z, 0, 0, 0, 0,
                          1);
    }

    static matrix4<t> diagonal(const vector4<t>& x)
    {
        return matrix4<t>(x.w, 0, 0, 0, 0, x.x, 0, 0, 0, 0, x.y, 0, 0, 0, 0,
                          x.z);
    }

    static matrix4<t> translation(t x, t y, t z)
    {
        return matrix4<t>(1, 0, 0, x, 0, 1, 0, y, 0, 0, 1, z, 0, 0, 0, 1);
    }

    static matrix4<t> translation(const vector3<t>& v)
    {
        return matrix4<t>(1, 0, 0, v.x, 0, 1, 0, v.y, 0, 0, 1, v.z, 0, 0, 0,
                          1);
    }

    static matrix4<t> cross(const vector3<t>& x)
    {
        return matrix4<t>(0, x.z, -x.y, 0, -x.z, 0, x.x, 0, x.y, x.x, 0, 0, 0,
                          0, 0, 0);
    }

private:
    storage m_;
};

//---------------------------------------------------------------------------

template <typename t>
const matrix4<t> operator*(const matrix4<t>& l, const matrix4<t>& r)
{
    return matrix4<t>(l(0, 0) * r(0, 0) + l(0, 1) * r(1, 0) + l(0, 2) * r(2, 0)
                      + l(0, 3) * r(3, 0),
                      l(0, 0) * r(0, 1) + l(0, 1) * r(1, 1) + l(0, 2) * r(2, 1)
                      + l(0, 3) * r(3, 1),
                      l(0, 0) * r(0, 2) + l(0, 1) * r(1, 2) + l(0, 2) * r(2, 2)
                      + l(0, 3) * r(3, 2),
                      l(0, 0) * r(0, 3) + l(0, 1) * r(1, 3) + l(0, 2) * r(2, 3)
                      + l(0, 3) * r(3, 3),

                      l(1, 0) * r(0, 0) + l(1, 1) * r(1, 0) + l(1, 2) * r(2, 0)
                      + l(1, 3) * r(3, 0),
                      l(1, 0) * r(0, 1) + l(1, 1) * r(1, 1) + l(1, 2) * r(2, 1)
                      + l(1, 3) * r(3, 1),
                      l(1, 0) * r(0, 2) + l(1, 1) * r(1, 2) + l(1, 2) * r(2, 2)
                      + l(1, 3) * r(3, 2),
                      l(1, 0) * r(0, 3) + l(1, 1) * r(1, 3) + l(1, 2) * r(2, 3)
                      + l(1, 3) * r(3, 3),

                      l(2, 0) * r(0, 0) + l(2, 1) * r(1, 0) + l(2, 2) * r(2, 0)
                      + l(2, 3) * r(3, 0),
                      l(2, 0) * r(0, 1) + l(2, 1) * r(1, 1) + l(2, 2) * r(2, 1)
                      + l(2, 3) * r(3, 1),
                      l(2, 0) * r(0, 2) + l(2, 1) * r(1, 2) + l(2, 2) * r(2, 2)
                      + l(2, 3) * r(3, 2),
                      l(2, 0) * r(0, 3) + l(2, 1) * r(1, 3) + l(2, 2) * r(2, 3)
                      + l(2, 3) * r(3, 3),

                      l(3, 0) * r(0, 0) + l(3, 1) * r(1, 0) + l(3, 2) * r(2, 0)
                      + l(3, 3) * r(3, 0),
                      l(3, 0) * r(0, 1) + l(3, 1) * r(1, 1) + l(3, 2) * r(2, 1)
                      + l(3, 3) * r(3, 1),
                      l(3, 0) * r(0, 2) + l(3, 1) * r(1, 2) + l(3, 2) * r(2, 2)
                      + l(3, 3) * r(3, 2),
                      l(3, 0) * r(0, 3) + l(3, 1) * r(1, 3) + l(3, 2) * r(2, 3)
                      + l(3, 3) * r(3, 3));
}

template <typename t>
const vector3<t> operator*(const matrix4<t>& l, const vector3<t>& r)
{
    return vector3<t>(
        l(0, 0) * r[0] + l(0, 1) * r[1] + l(0, 2) * r[2] + l(0, 3),
        l(1, 0) * r[0] + l(1, 1) * r[1] + l(1, 2) * r[2] + l(1, 3),
        l(2, 0) * r[0] + l(2, 1) * r[1] + l(2, 2) * r[2] + l(2, 3));
}

template <typename t>
const vector4<t> operator*(const matrix4<t>& l, const vector4<t>& r)
{
    return vector4<t>(
        l(0, 0) * r[0] + l(0, 1) * r[1] + l(0, 2) * r[2] + l(0, 3) * r[3],
        l(1, 0) * r[0] + l(1, 1) * r[1] + l(1, 2) * r[2] + l(1, 3) * r[3],
        l(2, 0) * r[0] + l(2, 1) * r[1] + l(2, 2) * r[2] + l(2, 3) * r[3],
        l(3, 0) * r[0] + l(3, 1) * r[1] + l(3, 2) * r[2] + l(3, 3) * r[3]);
}

template <typename t>
const matrix4<t> operator*(matrix4<t> l, t r)
{
    return l *= r;
}

//---------------------------------------------------------------------------

template <typename t>
matrix4<t> rotate_x(double angle)
{
    auto result = matrix4<t>::identity();
    result(1, 1) = std::cos(angle);
    result(2, 1) = -std::sin(angle);
    result(1, 2) = std::sin(angle);
    result(2, 2) = std::cos(angle);
    return result;
}

template <typename t>
matrix4<t> rotate_y(double angle)
{
    auto result = matrix4<t>::identity();
    result(0, 0) = std::cos(angle);
    result(2, 0) = -std::sin(angle);
    result(0, 2) = std::sin(angle);
    result(2, 2) = std::cos(angle);
    return result;
}

template <typename t>
matrix4<t> rotate_z(double angle)
{
    auto result = matrix4<t>::identity();
    result(0, 0) = std::cos(angle);
    result(1, 0) = -std::sin(angle);
    result(0, 1) = std::sin(angle);
    result(1, 1) = std::cos(angle);
    return result;
}

template <typename t>
matrix4<t> translate(matrix4<t> m, const vector3<t>& v)
{
    m(0, 3) += m(0, 0) * v.x + m(0, 1) * v.y + m(0, 2) * v.z;
    m(1, 3) += m(1, 0) * v.x + m(1, 1) * v.y + m(1, 2) * v.z;
    m(2, 3) += m(2, 0) * v.x + m(2, 1) * v.y + m(2, 2) * v.z;

    return m;
}

template <typename T>
matrix4<T> ortho(T w, T h)
{
    auto m = matrix4<T>::identity();
    m(0, 0) = T{2} / w;
    m(1, 1) = T{2} / h;
    m(2, 2) = -2;
    m(0, 3) = -1;
    m(1, 3) = -1;
    m(2, 3) = 1;

    return m;
}

} // namespace hexa

//---------------------------------------------------------------------------

namespace std
{

template <typename T>
ostream& operator<<(ostream& str, const hexa::matrix3<T>& m)
{
    str << '(' << m(0, 0) << ';' << m(0, 1) << ';' << m(0, 2) << ')' << '('
        << m(1, 0) << ';' << m(1, 1) << ';' << m(1, 2) << ')' << '(' << m(2, 0)
        << ';' << m(2, 1) << ';' << m(2, 2) << ')';

    str << std::endl << '[';
    for (auto i : m)
        str << i << ';';

    return str << ']';
}

template <typename T>
ostream& operator<<(ostream& str, const hexa::matrix4<T>& m)
{
    str << '(' << m(0, 0) << ';' << m(0, 1) << ';' << m(0, 2) << ';' << m(0, 3)
        << ')' << '(' << m(1, 0) << ';' << m(1, 1) << ';' << m(1, 2) << ';'
        << m(1, 3) << ')' << '(' << m(2, 0) << ';' << m(2, 1) << ';' << m(2, 2)
        << ';' << m(2, 3) << ')' << '(' << m(3, 0) << ';' << m(3, 1) << ';'
        << m(3, 2) << ';' << m(3, 3) << ')';

    str << std::endl << '[';
    for (auto i : m)
        str << i << ';';

    return str << ']';
}

} // namespace std
