//---------------------------------------------------------------------------
/// \file   hexa/client/gui/sfml2.hpp
/// \brief  Implementation for SFML 2.x
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

#include <unordered_map>
#include <SFML/Graphics.hpp>

#include "font_i.hpp"
#include "texture_i.hpp"
#include "renderer_i.hpp"

namespace gui
{

class sfml2_font : public font_i
{
public:
    sfml2_font(const sf::Font& font);

    float line_spacing(float point_size) const override;
    float ascend(float point_size) const override;
    float descend(float point_size) const override;

    const sf::Font& sf_font() const { return font_; }

private:
    struct info
    {
        int ascend;
        int descend;

        info()
            : ascend{0}
            , descend{0}
        {
        }
    };

    info metrics(float size) const;

    sf::Font font_;
    mutable std::unordered_map<float, info> cache_;
};

//---------------------------------------------------------------------------

class sfml2_texture : public texture_i
{
public:
    sfml2_texture(const sf::Texture& tex);
    const sf::Texture& sf_texture() const { return tex_; }
    pos size() const override;

private:
    sf::Texture tex_;
};

//---------------------------------------------------------------------------

class sfml2_renderer : public renderer_i
{
public:
    sfml2_renderer(sf::RenderTarget& target);
    virtual ~sfml2_renderer() {}

    void begin_paint() override;

    void end_paint() override;

    void clear(const color& background);

    void draw(const rect& box, const texture& fill) override;

    void draw(const rect& box, const color& fill) override;

    void draw(const pos& p, const texture& image) override;

    void scissor(const rect& area) override;

    void end_scissor() override;

public:
    int text_width(const std::u32string& text, const font& font,
                   float point_size = 16.f) override;

    texture make_text_label(const std::u32string& text, const color& fill,
                            const font& font, float point_size = 16.f,
                            text_style style = regular) override;

    texture load_texture(const std::string& name) override;

    font make_font(const std::string& name) override;

    virtual sf::Texture texture_resource(const std::string& name);
    virtual sf::Font font_resource(const std::string& name);

private:
    void flush();
    void add_vertex(pos vtx, const sf::Color& col);
    void add_vertex(pos vtx, sf::Vector2f uv);
    void set_primitive_type(sf::PrimitiveType type);
    void set_texture(const sf::Texture& tex);
    void disable_texture();

private:
    sf::RenderTarget& target_;
    sf::VertexArray buffer_;
    sf::RenderStates states_;
};

} // namespace gui
