//---------------------------------------------------------------------------
/// \file   client/gui/variable.hpp
/// \brief  Variables with callbacks and validation
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

#include <functional>
#include <string>
#include <boost/signals2.hpp>

namespace gui
{

class variable_base
{
public:
    virtual ~variable_base() {}
};

//---------------------------------------------------------------------------

template <typename T, typename V>
class variable : public variable_base
{
public:
    typedef V value_type;

public:
    variable(const V& init)
        : val_{init}
        , is_valid_{always_valid}
    {
    }

    variable(const V& init, std::function<bool(const V&)> is_valid)
        : val_{init}
        , is_valid_{is_valid}
    {
    }

    virtual ~variable() {}

    boost::signals2::signal<void(const V&)> on_change;

    const value_type& operator()() const { return val_; }

    bool can_set(const value_type& v) const { return is_valid_(v); }

    bool set(const value_type& v)
    {
        if (!can_set(v))
            return false;

        if (v != val_) {
            val_ = v;
            on_change(v);
        }
        return true;
    }

    T& operator=(const value_type& v)
    {
        if (!set(v))
            throw std::runtime_error("illegal value");

        return *this;
    }

    T& operator=(const T& v) { return operator=(v()); }

protected:
    value_type val_;
    std::function<bool(const value_type&)> is_valid_;

private:
    static constexpr bool always_valid(const value_type&) { return true; }
};

//---------------------------------------------------------------------------

class enumeration : public variable<enumeration, uint16_t>
{
public:
    enumeration(value_type v, value_type max)
        : variable(v, [=](const value_type& v) { return v < max_; })
        , max_(max)
    {
    }

    virtual ~enumeration() {}

    value_type max() const { return max_; }

protected:
    value_type max_;
};

//---------------------------------------------------------------------------

class integer : public variable<integer, int>
{
public:
    integer(value_type v)
        : variable{v}
    {
    }
};

//---------------------------------------------------------------------------

class real : public variable<real, double>
{
public:
    real(value_type v)
        : variable{v}
    {
    }
};

//---------------------------------------------------------------------------

} // namespace gui
