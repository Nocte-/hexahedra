//---------------------------------------------------------------------------
/// \file   client/gui/canvas.hpp
/// \brief  Base class for canvas implementations.
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

#include <string>
#include <vector>
#include <boost/signals2.hpp>
#include <hexa/basic_types.hpp>
#include <hexa/color.hpp>
#include "../texture.hpp"

namespace hexa {
namespace gui {

class font
{
public:
    virtual ~font() {}
};

class label
{
public:
    virtual ~label() {}

    virtual vector2<float> size() const = 0;
};

class canvas
{
public:
    typedef vector2<int>        point;
    typedef rectangle<int>      rect;

    typedef enum { tile, stretch } fill_type;

public:
    virtual ~canvas() { }

    point size() const { return size_; }

    virtual void update(double time_delta) = 0;

    virtual void clear(const color& background) = 0;

    virtual void draw(const rect& box, const texture& fill,
                    fill_type method = tile) = 0;

    virtual void draw(const rect& box, const color& fill) = 0;

    virtual void draw(const point& pos, std::shared_ptr<label> text) = 0;

    virtual std::shared_ptr<label> create_label
                    (const std::wstring& text, float point_size) = 0;

protected:
    point size_;
};

}} // namespace hexa::gui

