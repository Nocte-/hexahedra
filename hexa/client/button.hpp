//---------------------------------------------------------------------------
/// \file   client/button.hpp
/// \brief  SFML button class
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
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <functional>
#include <SFML/Graphics.hpp>
#include "sfml_resource_manager.hpp"

namespace hexa {

struct event;

class button
{
public:
    button(const std::wstring& label);
    button(int x, int y, const std::wstring& label);

    void draw(sf::RenderWindow& win);
    void process_event(const event& ev);

    std::function<void()> on_clicked;

    void set_position (int x, int y);

private:
    void on_mouse_enter();
    void on_mouse_leave();

private:
    sf::Text    label_;
    sf::Sprite  normal_;
    sf::Sprite  highlight_;
    sf::Sprite  select_;
    bool        is_hl_;
    int         x_, y_;
};

} // namespace hexa

