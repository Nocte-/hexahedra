//---------------------------------------------------------------------------
// client/resource_manager.cpp
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
// Copyright 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "resource_manager.hpp"

#include <cassert>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace boost::filesystem;

namespace hexa
{

extern boost::program_options::variables_map global_settings;

path resource_path(resource_type type)
{
    path base(global_settings["datadir"].as<std::string>());

    switch (type) {
    case res_block_texture:
        base /= "blocks";
        break;
    case res_model:
        base /= "models";
        break;
    case res_texture:
        base /= "textures";
        break;
    case res_image:
        base /= "img";
        break;
    case res_icon:
        base /= "icons";
        break;
    case res_sound_fx:
        base /= "sfx";
        break;
    case res_music:
        base /= "music";
        break;
    case res_ui:
        base /= "ui";
        break;
    case res_font:
        base /= "fonts";
        break;
    case res_shader:
        base /= "shaders";
        break;
    default:
        assert(false);
    };

    return base;
}

std::vector<std::string> resource_extensions(resource_type type)
{
    switch (type) {
    case res_block_texture:
        return {".png"};
    case res_model:
        return {".iqm"};
    case res_texture:
    case res_image:
    case res_icon:
        return {".png"};
    case res_sound_fx:
    case res_music:
        return {".ogg", ".mp3"};
    case res_ui:
        return {".png"};
    case res_font:
        return {".ttf", ".otf"};
    case res_shader:
        return {""};
    default:
        assert(false);
    };
    return {};
}

path resource_file(resource_type type, const std::string& name)
{
    for (auto& ext : resource_extensions(type)) {
        path file(resource_path(type) / (name + ext));
        if (exists(file))
            return file;
    }

    return path(resource_path(type) / name);
}

} // namespace hexa
