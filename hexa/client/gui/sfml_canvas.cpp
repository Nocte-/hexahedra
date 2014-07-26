//---------------------------------------------------------------------------
// client/gui/sfml_canvas.cpp
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

#include "sfml_canvas.hpp"

namespace hexa
{
namespace gui
{

namespace impl
{

class label : public gui::label, public sf::Text
{
public:
    label(const std::wstring& text, float point_size)
        : sf::Text(text, sf::Font(), (int)point_size)
    {
    }

    virtual ~label() {}

    vector2<float> size() const
    {
        auto rect(getGlobalBounds());
        return {rect.width, rect.height};
    }
};

} // namespace impl

inline sf::Color convert(const color& c)
{
    return sf::Color(c.r() * 255, c.g() * 255, c.b() * 255);
}

sfml_canvas::sfml_canvas(point size, const std::wstring& title)
    : win_(sf::VideoMode(size.x, size.y, 32), sf::String(title),
           sf::Style::Close, sf::ContextSettings(24, 8, 4, 3, 0))
{
}

sfml_canvas::~sfml_canvas()
{
}

void sfml_canvas::clear(const color& bg)
{
    win_.clear(convert(bg));
}

void sfml_canvas::draw(const rect& box, const color& bg)
{
    sf::RectangleShape shp(sf::Vector2f(box.width(), box.height()));
    shp.setPosition(box.first.x, box.first.y);
    shp.setFillColor(convert(bg));
    win_.draw(shp);
}

void sfml_canvas::draw(const rect& box, const texture& fill, fill_type method)
{
    (void)method;

    fill.bind();
    glColor4f(1, 1, 1, 1);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(box.first.x, box.first.y);
    glTexCoord2f(1, 0);
    glVertex2f(box.second.x, box.first.y);
    glTexCoord2f(1, 1);
    glVertex2f(box.second.x, box.second.y);
    glTexCoord2f(0, 1);
    glVertex2f(box.first.x, box.second.y);
    glEnd();

    fill.unbind();
}

void sfml_canvas::draw(const point& pos, std::shared_ptr<label> text)
{
    impl::label& sf_label(*reinterpret_cast<impl::label*>(text.get()));
    sf_label.setPosition(pos.x, pos.y);
    win_.draw(sf_label);
}

std::shared_ptr<label> sfml_canvas::create_label(const std::wstring& text,
                                                 float point_size)
{
    return std::shared_ptr<label>(new impl::label(text, point_size));
}
}
} // namespace hexa::gui
