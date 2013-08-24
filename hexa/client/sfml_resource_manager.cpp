//---------------------------------------------------------------------------
// client/sfml_resource_manager.cpp
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

#include "sfml_resource_manager.hpp"

#include <boost/program_options/variables_map.hpp>
#include <hexa/algorithm.hpp>
#include "iqm_loader.hpp"

namespace hexa {

extern boost::program_options::variables_map global_settings;

texture_manager textures;
sfml_image_manager   images;
sfml_texture_manager sfml_textures;
sprite_manager  sprites;
font_manager    fonts;
model_manager   models;
ui_elem_manager ui_elem;

std::string make_resource (const std::string& type, const std::string& file)
{
    boost::filesystem::path base (global_settings["datadir"].as<std::string>());
    return (base / type / file).string();
}

texture_manager::resource
texture_manager::load (const std::string& location)
{
    sf::Image img;
    if (!img.loadFromFile(resource_file(res_texture, location).string()))
        return nullptr;

    return std::make_shared<texture>(img, texture::transparent);
}

sfml_image_manager::resource
sfml_image_manager::load (const std::string& location)
{
    auto result (std::make_shared<sf::Texture>());
    if (result->loadFromFile(resource_file(res_image, location).string()))
        return result;

    return nullptr;
}

sfml_texture_manager::resource
sfml_texture_manager::load (const std::string& location)
{
    auto result (std::make_shared<sf::Texture>());
    if (!result->loadFromFile(resource_file(res_texture, location).string()))
        return nullptr;

    result->setSmooth(false);
    return result;
}

sprite_manager::resource
sprite_manager::load (const std::string& location)
{
    auto img_res (std::make_shared<sf::Image>());
    if (!img_res->loadFromFile(resource_file(res_icon, location).string()))
        return nullptr;

    auto tx (new sf::Texture);
    tx->loadFromImage(*img_res);
    auto result (std::make_shared<sf::Sprite>(*tx));
    result->setScale(2, 2);
    return result;
}

font_manager::resource
font_manager::load (const std::string& location)
{
    auto result (std::make_shared<sf::Font>());
    if (!result->loadFromFile(resource_file(res_font, location).string()))
        return nullptr;

    return result;
}

model_manager::resource
model_manager::load (const std::string& location)
{
    auto temp (iqm::load(resource_file(res_model, location)));
    //return std::make_shared<gl::model>(temp.first);
    gl::model* p (new gl::model(temp.first));
    return std::shared_ptr<gl::model>(p);
}

shader_manager::resource
shader_manager::load (const std::string& location)
{
    return std::make_shared<std::string>(file_contents(resource_file(res_shader, location)));
}

ui_elem_manager::resource
ui_elem_manager::load (const std::string& location)
{
    auto img_res (std::make_shared<sf::Image>());
    if (!img_res->loadFromFile(resource_file(res_ui, location).string()))
        return nullptr;

    auto tx (new sf::Texture);
    tx->loadFromImage(*img_res);
    tx->setSmooth(false);
    auto result (std::make_shared<sf::Sprite>(*tx));
    result->setScale(2, 2);
    return result;
}


} // namespace hexa

