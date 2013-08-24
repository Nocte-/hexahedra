//---------------------------------------------------------------------------
// client/button.cpp
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

#include "button.hpp"
#include "event.hpp"

namespace hexa {

template <typename t>
void center (t& obj)
{
    auto rect (obj.getLocalBounds());
    obj.setOrigin((int)rect.width * 0.5, (int)rect.height * 0.5);
}

button::button(const std::wstring& label)
    : button(0, 0, label)
{ }

button::button(int x, int y, const std::wstring& label)
    : is_hl_ (false)
    , x_ (x), y_ (y)
{
    label_.setString(label);
    label_.setFont(*fonts("armata"));
    label_.setCharacterSize(28);
    label_.setColor(sf::Color(255, 255, 255));
    label_.setPosition(x, y);

    normal_.setTexture(*images("button"));
    normal_.setPosition(x, y);

    highlight_.setTexture(*images("button_hl"));
    highlight_.setPosition(x, y);

    center(label_);
    center(normal_);
    center(highlight_);
}

void button::set_position (int x, int y)
{
    x_ = x; y_ = y;
    label_.setPosition(x, y - 2);
    normal_.setPosition(x, y);
    highlight_.setPosition(x, y);
}

void button::draw(sf::RenderWindow& win)
{
    win.draw(is_hl_ ? highlight_ : normal_);
    win.draw(label_);
}

void button::process_event (const event& ev)
{
    switch (ev.type)
    {
        case event::mouse_move_abs:
            {
            sf::Vector2f fmp (ev.xy.x, ev.xy.y);
            bool hl (normal_.getGlobalBounds().contains(fmp));
            if (hl != is_hl_)
            {
                if (hl)
                    on_mouse_enter();
                else
                    on_mouse_leave();
            }
            }
            break;

        case event::mouse_button_down:
            if (is_hl_)
                on_clicked();

        default:
            ;
    }
}

void button::on_mouse_enter()
{
    is_hl_ = true;
    label_.setColor(sf::Color(255, 255, 255));
}

void button::on_mouse_leave()
{
    is_hl_ = false;
    label_.setColor(sf::Color(240, 240, 240));
}

} // namespace hexa

