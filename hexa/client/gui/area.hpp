//---------------------------------------------------------------------------
/// \file   hexa/client/gui/area.hpp
/// \brief  Base class for all elements that occupy a rectangular area
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

#include "types.hpp"

namespace gui
{

class area_i
{
public:
    virtual ~area_i() {}
    virtual rect area() const = 0;
    virtual bool set_area(const rect&) { return false; }

    virtual pos fixed_size() const { return {0,0}; }
    virtual pos minimum_size() const { return {0,0}; }
};

class duplicate : public area_i
{
public:
    duplicate(area_i& a, area_i& b) : a_{a}, b_{b} {}

    virtual rect area() const override { return a_.area(); }

    virtual bool set_area(const rect& r) override
    {
        bool ra = a_.set_area(r);
        bool rb = b_.set_area(r);
        return ra && rb;
    }

private:
    area_i& a_;
    area_i& b_;
};

} // namespace gui
