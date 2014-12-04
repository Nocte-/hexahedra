//---------------------------------------------------------------------------
/// \file   hexa/client/gui/renderer_i.hpp
/// \brief  Interface class for renderers
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

#include <memory>
#include <string>
#include <vector>

#include "font_i.hpp"
#include "texture_i.hpp"
#include "types.hpp"

namespace gui
{

enum text_style { regular = 0, bold = 1, italic = 2, underlined = 4 };

class renderer_i
{
public:
    typedef std::unique_ptr<texture_i> texture;
    typedef std::unique_ptr<font_i> font;

public:
    virtual ~renderer_i() {}

    virtual void begin_paint(){};

    virtual void end_paint(){};

    virtual void clear(const color& background) = 0;

    virtual void draw(const rect& box, const texture& fill) = 0;

    virtual void draw(const rect& box, const color& fill) = 0;

    virtual void draw(const pos& p, const texture& image) = 0;

    virtual void scissor(const rect& area) = 0;

    virtual void end_scissor() = 0;

public:
    virtual int text_width(const std::u32string& text, const font& font,
                           float point_size = 16.f) = 0;

    virtual texture make_text_label(const std::u32string& text,
                                    const color& fill, const font& font,
                                    float point_size = 16.f,
                                    text_style style = regular) = 0;

    virtual texture load_texture(const std::string& filename) = 0;

    virtual font make_font(const std::string& filename) = 0;
};

} // namespace gui
