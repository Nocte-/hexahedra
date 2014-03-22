//---------------------------------------------------------------------------
/// \file   hexa/client/sfml_ogl3.hpp
/// \brief  SFML-based renderer that requires OpenGL 3.3
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
// Copyright 2013-2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <list>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>
#include <SFML/Window.hpp>

#include <hexa/basic_types.hpp>
#include <hexa/matrix.hpp>

#include "edge_buffer.hpp"
#include "opengl_vertex.hpp"
#include "shader.hpp"
#include "sfml.hpp"
#include "texture.hpp"
#include "types.hpp"
#include "vao.hpp"
#include "sfml_resource_manager.hpp"

namespace hexa {

class player;

typedef vertex_4< vtx_xyz<uint16_t>,        // Position
                  vtx_uv<uint8_t>,          // Texture coordinates
                  vtx_scalar<uint16_t>,     // Texture index
                  vtx_array<uint8_t, 2> >   // Light values, 4 nibbles

                  ogl3_terrain_vertex;

class sfml_ogl3 : public sfml
{
public:
    sfml_ogl3(sf::RenderWindow& win, scene& s);
    ~sfml_ogl3();

    void prepare(const player& plr);
    void opaque_pass();
    void transparent_pass();
    void handle_occlusion_queries();

    void sky_color(const color& col);
    void ambient_color(const color& col);
    void sun_color(const color& col);

    void load_textures(const std::vector<std::string>& name_list);

    std::unique_ptr<terrain_mesher_i>
         make_terrain_mesher();

    void draw(const gl::vbo& v) const;
    void draw_model(const wfpos& p, uint16_t m) const;

private:
    std::list<sf::Image> textures_;
    std::atomic<bool>    textures_ready_;

    texture_array texarr_;

    shader_program      terrain_shader_;
    uniform_variable    tex_;
    uniform_variable    fog_color_;
    uniform_variable    fog_density_;
    uniform_variable    ambient_light_;
    uniform_variable    sunlight_;
    uniform_variable    artificial_light_;

    gl::vbo             occlusion_block_;
    shader_program      model_shader_;

    model_manager::resource    mrfixit_;
};

} // namespace hexa

