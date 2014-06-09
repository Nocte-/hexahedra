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
#include "vbo.hpp"

namespace hexa
{

class player;

typedef vertex_4<vtx_xyz<int32_t>,      // Position
                 vtx_uv<uint8_t>,       // Texture coordinates
                 vtx_scalar<uint16_t>,  // Texture index
                 vtx_array<uint8_t, 2>> // Light values, 4 nibbles

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

    terrain_mesher_ptr make_terrain_mesher(vec3i offset);

    void draw(const gl::vbo& v, const matrix4<float>& mtx) override;
    void draw_model(const wfpos& p, uint16_t m) override;

protected:
    virtual std::string gl_id() const override { return "gl3"; }

private:
    std::list<sf::Image> textures_;
    std::atomic<bool> textures_ready_;
    texture_array texarr_;

    uniform_variable terrain_matrix_;
    uniform_variable terrain_camera_;
    uniform_variable tex_;
    uniform_variable fog_color_;
    uniform_variable fog_distance_;
    uniform_variable ambient_light_;
    uniform_variable sunlight_;
    uniform_variable artificial_light_;

    uniform_variable model_matrix_;
    uniform_variable model_camera_;
    uniform_variable model_tex_;

    struct animated_texture
    {
        unsigned int slice;
        unsigned int frame_count;
        gl::vbo buffer;

        animated_texture(unsigned int s, unsigned int f, gl::vbo&& b)
            : slice(s)
            , frame_count(f)
            , buffer(std::move(b))
        {
        }

#ifdef _MSC_VER
        animated_texture(const animated_texture&) = delete;
        animated_texture(animated_texture&& m)
            : slice(m.slice)
            , frame_count(m.frame_count)
            , buffer(std::move(m.buffer))
        {
        }
        animated_texture& operator=(animated_texture&& m)
        {
            if (this != &m) {
                slice = m.slice;
                frame_count = m.frame_count;
                buffer = std::move(m.buffer);
            }
            return *this;
        }
#endif
    };

    std::vector<animated_texture> animations_;
};

} // namespace hexa
