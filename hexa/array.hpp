//---------------------------------------------------------------------------
/// \file   array.hpp
/// \brief  2-D and 3-D array classes
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

#include <cassert>
#include <vector>

namespace hexa
{

template <typename T, size_t DimX, size_t DimY>
class array_2d : private std::vector<T>
{
    typedef typename std::vector<T> Impl;

public:
    typedef T value_type;
    typedef typename Impl::iterator iterator;
    typedef typename Impl::const_iterator const_iterator;
    typedef typename Impl::size_type size_type;

    using Impl::begin;
    using Impl::end;
    using Impl::operator[];

public:
    static_assert(DimX > 0 && DimY > 0, "Array size must be > 0.");

    array_2d() { resize(size()); }
    array_2d(const array_2d& m) = default;
    array_2d(array_2d&& m)
        : Impl{std::move(m)}
    {
    }

    array_2d& operator=(const array_2d&) = default;
    array_2d& operator=(array_2d&&) = default;

    array_2d(Impl&& m)
        : Impl{std::move(m)}
    {
        assert(Impl::size() == size());
    }

    constexpr size_type size() const { return DimX * DimY; }
    constexpr bool empty() const { return false; }
    void resize(size_type s)
    {
        assert(s == size());
        (void)s;
    }

    template <typename I>
    const value_type operator()(I x, I y) const
    {
        assert(x >= 0 && x < DimX);
        assert(y >= 0 && y < DimY);
        return operator[](x + DimX * y);
    }

    template <typename I>
    value_type& operator()(I x, I y)
    {
        assert(x >= 0 && x < DimX);
        assert(y >= 0 && y < DimY);
        return operator[](x + DimX * y);
    }

    template <typename Vector>
    const value_type operator()(const Vector& v) const
    {
        return operator()(v.x, v.y);
    }

    template <typename Vector>
    value_type& operator()(const Vector& v) const
    {
        return operator()(v.x, v.y);
    }

    template <typename I>
    const value_type get(I x, I y)
    {
        return operator()(x, y);
    }

    template <typename I>
    void set(I x, I y, value_type v)
    {
        operator()(x, y) = v;
    }

    void clear(value_type v = 0) { std::fill(begin(), end(), v); }

private:
    Impl elem_;
};

template <typename T, size_t DimX, size_t DimY, size_t DimZ>
class array_3d : private std::vector<T>
{
    typedef typename std::vector<T> Impl;

public:
    typedef T value_type;
    typedef typename Impl::iterator iterator;
    typedef typename Impl::const_iterator const_iterator;
    typedef typename Impl::size_type size_type;

    using Impl::begin;
    using Impl::end;
    using Impl::operator[];

public:
    static_assert(DimX > 0 && DimY > 0 && DimZ > 0, "Array size must be > 0.");

    array_3d() { resize(size()); }

    array_3d(array_3d&& m)
        : Impl{std::move(m)}
    {
    }

    array_3d(Impl&& m)
        : Impl{std::move(m)}
    {
        assert(Impl::size() == size());
    }

    constexpr size_type size() const { return DimX * DimY * DimZ; }
    constexpr bool empty() const { return false; }
    void resize(size_type s)
    {
        (void)s;
        assert(s == size());
    }

    template <typename I>
    const value_type operator()(I x, I y, I z) const
    {
        assert(x >= 0 && x < DimX);
        assert(y >= 0 && y < DimY);
        assert(z >= 0 && y < DimZ);
        return operator[](x + DimX * y + DimX * DimY * z);
    }

    template <typename I>
    value_type& operator()(I x, I y, I z)
    {
        assert(x >= 0 && x < DimX);
        assert(y >= 0 && y < DimY);
        assert(z >= 0 && y < DimZ);
        return operator[](x + DimX * y + DimX * DimY * z);
    }

    template <typename Vector>
    const value_type operator()(const Vector& v) const
    {
        return operator()(v.x, v.y, v.z);
    }

    template <typename Vector>
    value_type& operator()(const Vector& v) const
    {
        return operator()(v.x, v.y, v.z);
    }

    template <typename I>
    const value_type get(I x, I y, I z)
    {
        return operator()(x, y, z);
    }

    template <typename I>
    void set(I x, I y, I z, value_type v)
    {
        operator()(x, y, z) = v;
    }

    void clear(value_type v = 0) { std::fill(begin(), end(), v); }

private:
    Impl elem_;
};

} // namespace hexa
