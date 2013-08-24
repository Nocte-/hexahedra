//---------------------------------------------------------------------------
// client/texture.cpp
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
// Copyright 2012, 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "texture.hpp"

#include <stdexcept>
#include <boost/range.hpp>
#include <boost/algorithm/string.hpp>

#include "opengl.hpp"

using namespace boost::range;
using namespace boost::filesystem;

namespace hexa {

static const GLint GT2D  (GL_TEXTURE_2D);
static const GLint GT2DA (GL_TEXTURE_2D_ARRAY);
float texture::max_anisotropy_level_ = -1.0f;

void texture::init()
{
    assert(id_ == nullptr);
    id_ = std::make_shared<GLuint>(0);
    glCheck(glGenTextures(1, id_.get()));
    if (id() == 0)
        throw std::runtime_error("glGenTextures failed");
}

texture::texture()
{
}

texture::texture(const sf::Image& image, transparency_t alpha)
{
    init();
    load(image, alpha);
}

texture::texture(const path& image, transparency_t alpha)
{
    init();
    sf::Image img;
    if (!img.loadFromFile(image.string()))
        throw std::runtime_error("cannot read image");

    load(img, alpha);
}

texture::~texture()
{
    if (id_.unique())
        glCheck(glDeleteTextures(1, id_.get()));
}

float texture::anisotropy_level() const
{
    if (max_anisotropy_level_ == -1.0f)
    {
        std::string ext (reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));
        if (boost::find_first(ext, "GL_EXT_texture_filter_anisotropic"))
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy_level_);
        else
            max_anisotropy_level_ = 0.0f;
    }

    return max_anisotropy_level_;
}

void texture::load(const sf::Image& image, transparency_t alpha)
{
    if (id_ == nullptr)
        init();

    assert(image.getSize().x > 0 && image.getSize().y > 0);
    set_parameters(GT2D);

    GLint internal_format (alpha == transparent? GL_RGBA : GL_RGB);
    glCheck(glTexImage2D(GT2D, 0, internal_format,
                 image.getSize().x, image.getSize().y,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, image.getPixelsPtr()));

    if (glGenerateMipmap)
        glCheck(glGenerateMipmap(GT2D));
}

void texture::set_parameters(GLint type)
{
    glCheck(glBindTexture(type, id()));

    glCheck(glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_REPEAT));

    glCheck(glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    if (glGenerateMipmap)
    {
        glCheck(glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        float al (anisotropy_level());
        if (al > 0.f)
        {
            glCheck(glTexParameterf(type, GL_TEXTURE_MAX_ANISOTROPY_EXT, al));
            glCheck(glTexParameteri(type, GL_TEXTURE_MAX_LEVEL, 4));
        }
    }
    else
    {
        glCheck(glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    }
}


/////////////////////////////////////////////////////////////////////////////

texture_array::texture_array(const std::list<sf::Image>& imagelist,
                             uint16_t width, uint16_t height,
                             transparency_t alpha)
{
    width_ = width;
    height_ = height;
    init();
    load(imagelist, width, height, alpha);
}

void texture_array::load(const std::list<sf::Image>& imagelist,
                         uint16_t width, uint16_t height,
                         transparency_t alpha)
{
    width_ = width;
    height_ = height;

    if (imagelist.empty())
        return;

    if (id_ == nullptr)
        init();

    set_parameters(GT2DA);

    GLint internal_format (alpha == transparent? GL_RGBA : GL_RGB);
    glCheck(glTexImage3D(GT2DA, 0, internal_format,
                 width_, height_,
                 imagelist.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0));

    unsigned int slice (0);
    for(const auto& img : imagelist)
    {
        glCheck(glTexSubImage3D(GT2DA, 0, 0, 0, slice, width_, height_,
                                1, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr()));
        ++slice;
    }

    if (glGenerateMipmap)
        glCheck(glGenerateMipmap(GT2DA));
}

void texture_array::load(const sf::Image& img, unsigned int index,
                         unsigned int part)
{
    if (id_ == nullptr)
    {
        init();
        set_parameters(GT2DA);
    }
    else
    {
        glCheck(glBindTexture(GT2DA, id()));
    }

    part %= img.getSize().y / height_;
    glCheck(glTexSubImage3D(GT2DA, 0, 0, 0, index, width_, height_,
                            1, GL_RGBA, GL_UNSIGNED_BYTE,
                            img.getPixelsPtr() + width_ * 4 * part));

    if (glGenerateMipmap)
        glCheck(glGenerateMipmap(GT2DA));
}

} // namespace hexa

