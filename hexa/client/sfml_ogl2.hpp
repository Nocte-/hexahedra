//---------------------------------------------------------------------------
/// \file   hexa/client/sfml_ogl2.hpp
/// \brief  OpenGL 2.1 render backend based on SFML.
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

#include <GL/glew.h>
#include <GL/gl.h>
#include <SFML/Window.hpp>

#include <hexa/basic_types.hpp>
#include <hexa/matrix.hpp>

#include "opengl_vertex.hpp"
#include "shader.hpp"
#include "sfml.hpp"
#include "texture.hpp"
#include "vbo.hpp"

namespace hexa {

typedef vertex_4< vtx_xyz<float>,     // Position
                  vtx_uv<float>,      // Texture coordinates
                  vtx_rgb<uint8_t>,   // RGB light
                  vtx_padding<1>>     // Round to 8 bytes

                  ogl2_terrain_vertex;

class sfml_ogl2 : public sfml
{
public:
    /** Constructor. */
    sfml_ogl2(sf::RenderWindow& win, scene& s);
    ~sfml_ogl2();

    terrain_mesher_ptr make_terrain_mesher(vec3i offset) override;

    void prepare(const player& plr);
    void opaque_pass();
    void transparent_pass();
    void handle_occlusion_queries();

    void draw(const gl::vbo& v, const matrix4<float>& mtx) override;
    void draw_model(const wfpos& p, uint16_t m) override;

    void sun_color(const color&);
    void sky_color(const color&);
    void ambient_color(const color&);

    void load_textures(const std::vector<std::string>& name_list);

protected:
    virtual std::string gl_id() const override
        { return "gl2"; }

private:
    sf::Image temp_img_;
    texture   texture_atlas_;
    bool      textures_ready_;
};

} // namespace hexa

