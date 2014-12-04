//---------------------------------------------------------------------------
// client/gui/sfml2.cpp
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
#include "sfml2.hpp"

#include <iostream>

namespace gui
{

inline sf::Color col(color c)
{
    c *= 255;
    return {(uint8_t)c[0], (uint8_t)c[1], (uint8_t)c[2], (uint8_t)c[3]};
}

sfml2_font::sfml2_font(const sf::Font& font)
    : font_{font}
{
}

sfml2_font::info sfml2_font::metrics(float size) const
{
    auto found = cache_.find(size);
    if (found != cache_.end())
        return found->second;

    info m;

    // Scan through the Latin glyphs
    for (sf::Uint32 i = 0x0020; i < 0x02B0; ++i) {
        const auto& glyph = font_.getGlyph(i, size, false);

        std::cout << "Glyph " << i << " : " << glyph.bounds.height << " "
                  << glyph.bounds.top << std::endl;

        m.descend
            = std::max<int>(m.descend, glyph.bounds.height + glyph.bounds.top);
        m.ascend = std::max<int>(m.ascend, -glyph.bounds.top);
    }
    cache_[size] = m;
    return m;
}

float sfml2_font::line_spacing(float point_size) const
{
    return font_.getLineSpacing(point_size);
}

float sfml2_font::ascend(float point_size) const
{
    return metrics(point_size).ascend;
}

float sfml2_font::descend(float point_size) const
{
    return metrics(point_size).descend;
}

//---------------------------------------------------------------------------

sfml2_texture::sfml2_texture(const sf::Texture& tex)
    : tex_{tex}
{
}

pos sfml2_texture::size() const
{
    auto s = tex_.getSize();
    return {s.x, s.y};
}

//---------------------------------------------------------------------------

sfml2_renderer::sfml2_renderer(sf::RenderTarget& target)
    : target_{target}
{
}

void sfml2_renderer::begin_paint()
{
}

void sfml2_renderer::end_paint()
{
    flush();
}

void sfml2_renderer::clear(const color& background)
{
    target_.clear(col(background));
}

void sfml2_renderer::draw(const rect& box, const texture& fill)
{
    auto& sftext = dynamic_cast<const sfml2_texture&>(*fill);
    set_primitive_type(sf::Triangles);
    set_texture(sftext.sf_texture());

    add_vertex(box.top_left(), sf::Vector2f{0, 0});
    add_vertex(box.top_right(), sf::Vector2f{1, 0});
    add_vertex(box.bottom_right(), sf::Vector2f{1, 1});

    add_vertex(box.top_left(), sf::Vector2f{0, 0});
    add_vertex(box.bottom_right(), sf::Vector2f{1, 1});
    add_vertex(box.bottom_left(), sf::Vector2f{0, 1});
}

void sfml2_renderer::draw(const rect& box, const color& fill)
{
    set_primitive_type(sf::Triangles);
    disable_texture();
    auto color = col(fill);

    add_vertex(box.top_left(), color);
    add_vertex(box.bottom_right(), color);
    add_vertex(box.top_right(), color);

    add_vertex(box.top_left(), color);
    add_vertex(box.bottom_right(), color);
    add_vertex(box.bottom_left(), color);
}

void sfml2_renderer::draw(const pos& p, const texture& image)
{
    auto& tex = dynamic_cast<const sfml2_texture&>(*image);
    sf::Sprite sprite{tex.sf_texture()};
    sprite.setPosition(p.x, p.y);
    target_.draw(sprite);
}

void sfml2_renderer::scissor(const rect& area)
{
}

void sfml2_renderer::end_scissor()
{
}

int sfml2_renderer::text_width(const std::u32string& text, const font& font,
                               float point_size)
{
    const sfml2_font& tmp = dynamic_cast<const sfml2_font&>(*font);
    const sf::Font& sffont = tmp.sf_font();

    int width = 0;
    sf::Uint32 prev = 0;
    for (sf::Uint32 c : text) {
        width += sffont.getKerning(prev, c, point_size);
        width += sffont.getGlyph(c, point_size, false).advance;
        prev = c;
    }
    return width;
}

renderer_i::texture sfml2_renderer::make_text_label(const std::u32string& text,
                                                    const color& fill,
                                                    const font& font,
                                                    float point_size,
                                                    text_style style)
{
    const sfml2_font& tmp = dynamic_cast<const sfml2_font&>(*font);
    const sf::Font& sffont = tmp.sf_font();

    sf::String str(reinterpret_cast<const sf::Uint32*>(text.c_str()));
    sf::Text label(str, sffont, (int)point_size);
    sf::RenderTexture rt;

    auto size = label.getLocalBounds();
    if (!rt.create(size.width + 1, font->height(point_size) + 1))
        throw std::runtime_error("cannot create sf::RenderTexture");

    rt.clear(sf::Color::Transparent);
    label.move(0, -label.getLocalBounds().top);
    label.setColor(col(fill));
    label.setStyle(static_cast<sf::Uint32>(style));
    rt.draw(label);
    rt.display();
    return texture{new sfml2_texture{rt.getTexture()}};
}

renderer_i::texture sfml2_renderer::load_texture(const std::string& name)
{
    return texture{new sfml2_texture{texture_resource(name)}};
}

sf::Texture sfml2_renderer::texture_resource(const std::string &name)
{
    sf::Texture result;
    result.loadFromFile(name);
    return result;
}

renderer_i::font sfml2_renderer::make_font(const std::string& name)
{
    return font{new sfml2_font{font_resource(name)}};
}

sf::Font sfml2_renderer::font_resource(const std::string &name)
{
    sf::Font result;
    result.loadFromFile(name);
    return result;
}

void sfml2_renderer::flush()
{
    if (buffer_.getVertexCount() > 0) {
        target_.draw(buffer_, states_);
        buffer_.clear();
    }
}

void sfml2_renderer::add_vertex(pos vtx, const sf::Color& col)
{
    buffer_.append({{(float)vtx[0], (float)vtx[1]}, col});
}

void sfml2_renderer::add_vertex(pos vtx, sf::Vector2f uv)
{
    buffer_.append({{(float)vtx[0], (float)vtx[1]}, uv});
}

void sfml2_renderer::set_primitive_type(sf::PrimitiveType type)
{
    if (buffer_.getPrimitiveType() != type) {
        flush();
        buffer_.setPrimitiveType(type);
    }
}

void sfml2_renderer::set_texture(const sf::Texture& tex)
{
    if (states_.texture != &tex) {
        flush();
        states_.texture = &tex;
    }
}

void sfml2_renderer::disable_texture()
{
    if (states_.texture != nullptr) {
        flush();
        states_.texture = nullptr;
    }
}

} // namespace gui
