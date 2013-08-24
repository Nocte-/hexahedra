//---------------------------------------------------------------------------
/// \file   client/gui/sfml_canvas.hpp
/// \brief  Canvas implemented using SFML
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

#include "canvas.hpp"

namespace hexa {
namespace gui {

class sfml_canvas : public canvas
{
public:
    sfml_canvas (point size, const std::wstring& title);

    ~sfml_canvas();

    virtual void clear (const color& background);

    virtual void draw (const rect& box, const texture& fill,
                       fill_type method = tile);

    virtual void draw (const rect& box, const color& fill);

    virtual void draw (const point& pos, std::shared_ptr<label> text);

    virtual std::shared_ptr<label> create_label
                    (const std::wstring& text, float point_size);

private:
    sf::RenderWindow win_;
};

}} // namespace hexa::gui

