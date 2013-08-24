//---------------------------------------------------------------------------
/// \file   client/texture.hpp
/// \brief  Simple OpenGL texture wrapper.
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
// Copyright 2012-2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <cassert>
#include <list>
#include <memory>
#include <boost/filesystem/path.hpp>

#include <GL/glew.h>
#include <GL/gl.h>
#include <SFML/Graphics.hpp>
#include <hexa/basic_types.hpp>
#include <hexa/rectangle.hpp>

#include "opengl.hpp"

namespace hexa {

class texture
{
public:
    typedef enum
    {
        transparent,
        opaque
    }
    transparency_t;

public:
    texture();

    texture(const sf::Image& image,
            transparency_t alpha = opaque);

    texture(const boost::filesystem::path& image,
            transparency_t alpha = opaque);

    virtual ~texture();

    operator bool() const { return id_ != nullptr; }

    GLuint id() const { assert(id_); return *id_; }

    void bind() const { glCheck(glBindTexture(GL_TEXTURE_2D, id())); }

    static void unbind() { glCheck(glBindTexture(GL_TEXTURE_2D, 0)); }

    void load(const sf::Image& image, transparency_t alpha);
    void load(const boost::filesystem::path& image, transparency_t alpha);

protected:
    float anisotropy_level() const;

    void init();
    void set_parameters(GLint type);

    std::shared_ptr<GLuint> id_;
    static float max_anisotropy_level_;
};

class texture_array : public texture
{
public:
    texture_array()
        : texture()
        , width_(0)
        , height_(0)
    { }

    texture_array(const std::list<sf::Image>& imagelist,
                  uint16_t width, uint16_t height,
                  transparency_t alpha = opaque);

    void bind() const { glBindTexture(GL_TEXTURE_2D_ARRAY, id()); }

    static void unbind() { glBindTexture(GL_TEXTURE_2D_ARRAY, 0); }

    void load(const std::list<sf::Image>& imagelist,
              uint16_t width, uint16_t height, transparency_t alpha);

    void load(const sf::Image& img, unsigned int index, unsigned int part = 0);

private:
    uint16_t    width_;
    uint16_t    height_;
};

} // namespace hexa

