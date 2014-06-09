//---------------------------------------------------------------------------
/// \file   client/sfml_resource_manager.hpp
/// \brief  Loading and caching resources using SFML.
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

#include <GL/glew.h>
#include <SFML/Graphics.hpp>

#include "model_opengl.hpp"
#include "resource_manager.hpp"
#include "texture.hpp"

namespace hexa
{

std::string make_resource(const std::string& type, const std::string& file);

class texture_manager : public resource_manager<hexa::texture>
{
protected:
    virtual resource load(const std::string& location);
};

class sfml_image_manager : public resource_manager<sf::Texture>
{
protected:
    virtual resource load(const std::string& location);
};

class sfml_texture_manager : public resource_manager<sf::Texture>
{
protected:
    virtual resource load(const std::string& location);
};

class sprite_manager : public resource_manager<sf::Sprite>
{
protected:
    virtual resource load(const std::string& location);
};

class font_manager : public resource_manager<sf::Font>
{
protected:
    virtual resource load(const std::string& location);
};

class model_manager : public resource_manager<gl::model>
{
protected:
    virtual resource load(const std::string& location);
};

class shader_manager : public resource_manager<std::string>
{
protected:
    virtual resource load(const std::string& location);
};

class ui_elem_manager : public resource_manager<sf::Sprite>
{
protected:
    virtual resource load(const std::string& location);
};

// Declared globally; we don't need a singleton in this case
extern texture_manager textures;
extern sfml_texture_manager sfml_textures;
extern sfml_image_manager images;
extern sprite_manager sprites;
extern font_manager fonts;
extern model_manager models;
extern shader_manager shaders;
extern ui_elem_manager ui_elem;

} // namespace hexa
