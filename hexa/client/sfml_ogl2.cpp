//---------------------------------------------------------------------------
// hexa/client/sfml_ogl2.cpp
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
#include "sfml_ogl2.hpp"

#include <list>
#include <stdexcept>
#include <sstream>
#include <random>
#include <unordered_map>

#include <boost/range/algorithm.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/thread/locks.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>

#include <hexa/basic_types.hpp>
#include <hexa/block_types.hpp>
#include <hexa/frustum.hpp>
#include <hexa/voxel_algorithm.hpp>
#include <hexa/voxel_range.hpp>

#include "game.hpp"
#include "scene.hpp"
#include "texture.hpp"
#include "occlusion_query.hpp"
#include "opengl.hpp"
#include "opengl_vertex.hpp"
#include "sfml_resource_manager.hpp"

using namespace boost;
using namespace boost::range;
using namespace boost::adaptors;
using namespace boost::numeric::ublas;

namespace fs = boost::filesystem;

namespace hexa {

namespace {

typedef vertex_1<vtx_xyz<int16_t> > occ_cube_vtx;

} // anonymous namespace

class terrain_mesher_ogl2 : public terrain_mesher_i
{
public:
	terrain_mesher_ogl2(vec3i offset)
		: terrain_mesher_i{offset}
		, empty_{true}
    { }

    static vector3<uint8_t> light_to_rgb (light l)
    {
        vector temp (0, 0, 0);
        temp  = vector(0.5f, 0.6f, 1.0f) * 0.7f * (float(l.ambient)  / 15.f);
        temp += vector(1.0f, 1.0f, 0.7f) * 0.9f * (float(l.sunlight) / 15.f);
        temp += vector(1.0f, 1.0f, 0.6f) * 0.3f * (float(l.artificial) / 15.f);

        const float gamma (0.7f);
        vector3<uint8_t> rgb (std::min(255.f, 255 * std::pow(temp[0], gamma)),
                              std::min(255.f, 255 * std::pow(temp[1], gamma)),
                              std::min(255.f, 255 * std::pow(temp[2], gamma)));

        return rgb;
    }

    void add_face(chunk_index p, direction_type side,
                  uint16_t texture, light l)
    {
        static const int8_t offsets[6][4][3] =
            { { {1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1} },
              { {0, 1, 1}, {0, 1, 0}, {0, 0, 0}, {0, 0, 1} },
              { {1, 1, 1}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1} },
              { {0, 0, 1}, {0, 0, 0}, {1, 0, 0}, {1, 0, 1} },
              { {0, 1, 1}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1} },
              { {0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0} } };

        const int8_t(*o)[3] = offsets[side];


        empty_ = false;

        float u0 ((float(texture % 16)) / 16.f + 1.f/64.f);
        float u1 (u0 + 1.f/32.f);

        float v0 ((float(texture / 16)) / 16.f + 1.f/64.f);
        float v1 (v0 + 1.f/32.f);

        vector3<uint8_t> rgb (light_to_rgb(l));

        data_.emplace_back(ogl2_terrain_vertex
            (vector3<float>(p.x+o[0][0], p.y+o[0][1], p.z+o[0][2]) * 16.f,
             vector2<float>(u0, v0), rgb));

        data_.emplace_back(ogl2_terrain_vertex
            (vector3<float>(p.x+o[1][0], p.y+o[1][1], p.z+o[1][2]) * 16.f,
             vector2<float>(u0, v1), rgb));

        data_.emplace_back(ogl2_terrain_vertex
            (vector3<float>(p.x+o[2][0], p.y+o[2][1], p.z+o[2][2]) * 16.f,
             vector2<float>(u1, v1), rgb));

        data_.emplace_back(ogl2_terrain_vertex
            (vector3<float>(p.x+o[3][0], p.y+o[3][1], p.z+o[3][2]) * 16.f,
             vector2<float>(u1, v0), rgb));
    }


    void add_custom_block (chunk_index i, const custom_block& model, const std::vector<light>& l)
    {
        empty_ = false;

        vector3<float> offset (i);
        offset *= chunk_size;

        for (auto& part : model)
        {
            auto bbox (part.bounding_box());
            const auto& a (bbox.first);
            const auto& b (bbox.second);

            // Face: +x
            float u0 ((float(part.textures[0] % 16)) / 16.f + 1.f/64.f);
            float v0 ((float(part.textures[0] / 16)) / 16.f + 1.f/64.f);
            vector3<uint8_t> rgb (light_to_rgb(l[0]));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(b.x, a.y, a.z) + offset,
                 vector2<float>(u0 + b.y / 512.f, v0 + b.z / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(b.x, b.y, a.z) + offset,
                 vector2<float>(u0 + a.y / 512.f, v0 + b.z / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(b.x, b.y, b.z) + offset,
                 vector2<float>(u0 + a.y / 512.f, v0 + a.z / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(b.x, a.y, b.z) + offset,
                 vector2<float>(u0 + b.y / 512.f, v0 + a.z / 512.f), rgb));

            // Face: -x
            u0 = (float(part.textures[1] % 16)) / 16.f + 1.f/64.f;
            v0 = (float(part.textures[1] / 16)) / 16.f + 1.f/64.f;
            rgb = light_to_rgb(l[1]);

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(a.x, b.y, a.z) + offset,
                 vector2<float>(u0 + a.y / 512.f, v0 + b.z / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(a.x, a.y, a.z) + offset,
                 vector2<float>(u0 + b.y / 512.f, v0 + b.z / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(a.x, a.y, b.z) + offset,
                 vector2<float>(u0 + b.y / 512.f, v0 + a.z / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(a.x, b.y, b.z) + offset,
                 vector2<float>(u0 + a.y / 512.f, v0 + a.z / 512.f), rgb));

            // Face: +y
            u0 = (float(part.textures[2] % 16)) / 16.f + 1.f/64.f;
            v0 = (float(part.textures[2] / 16)) / 16.f + 1.f/64.f;
            rgb = light_to_rgb(l[2]);

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(b.x, b.y, b.z) + offset,
                 vector2<float>(u0 + a.x / 512.f, v0 + a.z / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(b.x, b.y, a.z) + offset,
                 vector2<float>(u0 + a.x / 512.f, v0 + b.z / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(a.x, b.y, a.z) + offset,
                 vector2<float>(u0 + b.x / 512.f, v0 + b.z / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(a.x, b.y, b.z) + offset,
                 vector2<float>(u0 + b.x / 512.f, v0 + a.z / 512.f), rgb));

            // Face: -y
            u0 = (float(part.textures[3] % 16)) / 16.f + 1.f/64.f;
            v0 = (float(part.textures[3] / 16)) / 16.f + 1.f/64.f;
            rgb = light_to_rgb(l[3]);

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(a.x, a.y, a.z) + offset,
                 vector2<float>(u0 + b.x / 512.f, v0 + b.z / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(b.x, a.y, a.z) + offset,
                 vector2<float>(u0 + a.x / 512.f, v0 + b.z / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(b.x, a.y, b.z) + offset,
                 vector2<float>(u0 + a.x / 512.f, v0 + a.z / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(a.x, a.y, b.z) + offset,
                 vector2<float>(u0 + b.x / 512.f, v0 + a.z / 512.f), rgb));

            // Face: +z
            u0 = (float(part.textures[4] % 16)) / 16.f + 1.f/64.f;
            v0 = (float(part.textures[4] / 16)) / 16.f + 1.f/64.f;
            rgb = light_to_rgb(l[4]);

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(a.x, a.y, b.z) + offset,
                 vector2<float>(u0 + a.x / 512.f, v0 + a.y / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(b.x, a.y, b.z) + offset,
                 vector2<float>(u0 + b.x / 512.f, v0 + a.y / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(b.x, b.y, b.z) + offset,
                 vector2<float>(u0 + b.x / 512.f, v0 + b.y / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(a.x, b.y, b.z) + offset,
                 vector2<float>(u0 + a.x / 512.f, v0 + b.y / 512.f), rgb));

            // Face: -z
            u0 = (float(part.textures[5] % 16)) / 16.f + 1.f/64.f;
            v0 = (float(part.textures[5] / 16)) / 16.f + 1.f/64.f;
            rgb = light_to_rgb(l[5]);

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(a.x, b.y, a.z) + offset,
                 vector2<float>(u0 + a.x / 512.f, v0 + b.y / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(b.x, b.y, a.z) + offset,
                 vector2<float>(u0 + b.x / 512.f, v0 + b.y / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(b.x, a.y, a.z) + offset,
                 vector2<float>(u0 + b.x / 512.f, v0 + a.y / 512.f), rgb));

            data_.emplace_back(ogl2_terrain_vertex
                (vector3<float>(a.x, a.y, a.z) + offset,
                 vector2<float>(u0 + a.x / 512.f, v0 + a.y / 512.f), rgb));
        }
    }

    bool empty() const { return empty_; }

    gl::vbo make_buffer() const
    {
        return gl::make_vbo(data_);
    }

public:
    bool empty_;
    std::vector<ogl2_terrain_vertex> data_;
};

//---------------------------------------------------------------------------

sfml_ogl2::sfml_ogl2(sf::RenderWindow& app, scene& s)
    : sfml{app, s}
	, textures_ready_{false}
{
    static const int16_t cube[24*3] = {
        0  , 0  , 0   ,   256, 0  , 0   ,   256, 256, 0   ,   0  , 256, 0   ,
        256, 0  , 0   ,   256, 0  , 256 ,   256, 256, 256 ,   256, 256, 0   ,
        0  , 256, 256 ,   256, 256, 256 ,   256, 0  , 256 ,   0  , 0  , 256 ,
        0  , 256, 0   ,   0  , 256, 256 ,   0  , 0  , 256 ,   0  , 0  , 0   ,
        0  , 0  , 0   ,   256, 0  , 0   ,   256, 0  , 256 ,   0  , 0  , 256 ,
        0  , 256, 256 ,   256, 256, 256 ,   256, 256, 0   ,   0  , 256, 0    };

    occlusion_block_ = gl::vbo(cube, 24, sizeof(occ_cube_vtx));

    load_shader(terrain_shader_, "terrain_" + gl_id());
    terrain_shader_.bind_attribute(0, "position");
    terrain_shader_.bind_attribute(1, "uv");
    terrain_shader_.bind_attribute(2, "color");

    load_shader(model_shader_, "model_" + gl_id());

}

sfml_ogl2::~sfml_ogl2()
{
}

renderer_i::terrain_mesher_ptr sfml_ogl2::make_terrain_mesher(vec3i offset)
{
	return std::unique_ptr<terrain_mesher_i>(new terrain_mesher_ogl2(offset));
}

void sfml_ogl2::load_textures(const std::vector<std::string>& name_list)
{
    textures_ready_ = false;

    sf::Image atlas;
    temp_img_.create(512, 512, sf::Color::Transparent);
	fs::path unknown_file {resource_file(res_block_texture, "unknown")};

    unsigned int x =0, y =0;

    for (auto& name : name_list)
    {
		fs::path file {resource_file(res_block_texture, name)};

        sf::Image tile;
        if (fs::is_regular_file(file))
        {
            tile.loadFromFile(file.string());
        }
        else
        {
			std::string clip {name.begin(), find_last(name, ".").begin()};

			fs::path clipfile {resource_file(res_block_texture, clip)};
            if (fs::is_regular_file(clipfile))
                tile.loadFromFile(clipfile.string());
            else
                tile.loadFromFile(unknown_file.string());
        }

        // Add an 8-pixel edge around every texture to prevent seaming
        // when mipmapping is turned on.
        temp_img_.copy(tile, x, y, sf::IntRect(8, 8, 8, 8), true);
        temp_img_.copy(tile, x + 8, y, sf::IntRect(0, 8, 16, 8), true);
        temp_img_.copy(tile, x + 24, y, sf::IntRect(0, 8, 8, 8), true);

        temp_img_.copy(tile, x, y + 8 , sf::IntRect(8, 0, 8, 16), true);
        temp_img_.copy(tile, x + 8, y + 8, sf::IntRect(0, 0, 16, 16), true);
        temp_img_.copy(tile, x + 24, y + 8 , sf::IntRect(0, 0, 8, 16), true);

        temp_img_.copy(tile, x, y + 24, sf::IntRect(8, 0, 8, 8), true);
        temp_img_.copy(tile, x + 8, y + 24, sf::IntRect(0, 0, 16, 8), true);
        temp_img_.copy(tile, x + 24, y + 24, sf::IntRect(0, 0, 8, 8), true);

        x += 32;
        if (x >= 512)
        {
            x = 0;
            y += 32;
        }
    }

    //temp_img_.saveToFile("/tmp/test.png");
    textures_ready_ = true;
}

void sfml_ogl2::sky_color(const color& c)
{
    glClearColor(c.r(), c.g(), c.b(), 1.0);
}

void sfml_ogl2::sun_color(const color&)
{
}

void sfml_ogl2::ambient_color(const color&)
{
}

void sfml_ogl2::prepare(const player& plr)
{
    if (textures_ready_)
    {
        texture_atlas_.load(temp_img_, texture::transparent);
        textures_ready_ = false;
    }

    sky_color(color(0.56f, 0.67f, 1.0f));
    sfml::prepare(plr);
}

void sfml_ogl2::opaque_pass()
{
    static const float fog_color[4] = { 0.56f, 0.67f, 1.0f, 1.0f };

    if (!texture_atlas_)
        return;

    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_EXP2);
    glFogfv(GL_FOG_COLOR, fog_color);
    glFogf(GL_FOG_DENSITY, 2.2f / (float)(scene_.view_distance() * chunk_size * 4));
    glHint(GL_FOG_HINT, GL_NICEST);

    frustum clip (camera_.mvp_matrix());
    const float sphere_diam (16.f * 13.86f);

    glEnable(GL_TEXTURE_2D);
    texture_atlas_.bind();

    scene_.for_each_opaque_vbo([&](const chunk_coordinates& pos, const gl::vbo& vbo)
    {
        vec3f offset (vec3i(pos - chunk_offset_));
        offset *= 256.f;

		if (clip.is_inside(offset + vec3f{128,128,128}, sphere_diam))
        {
            auto mtx = translate(camera_.model_view_matrix(), offset);
            glLoadMatrixf(mtx.as_ptr());
            vbo.bind();
            bind_attributes<ogl2_terrain_vertex>();
            vbo.draw();
        }
    });

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_FOG);
}

void sfml_ogl2::transparent_pass()
{
    if (!texture_atlas_)
        return;

    frustum clip (camera_.mvp_matrix());
    const float sphere_diam (16.f * 13.86f);

    glEnable(GL_FOG);
    glEnable(GL_TEXTURE_2D);
    texture_atlas_.bind();

    scene_.for_each_transparent_vbo([&](const chunk_coordinates& pos, const gl::vbo& vbo)
    {
        vec3f offset (vec3i(pos - chunk_offset_));
        offset *= 256.f;

		if (clip.is_inside(offset + vec3f{128,128,128}, sphere_diam))
        {
            auto mtx = translate(camera_.model_view_matrix(), offset);
            glLoadMatrixf(mtx.as_ptr());
            vbo.bind();
            bind_attributes<ogl2_terrain_vertex>();
            vbo.draw();
        }
    });

    gl::vbo::unbind();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_FOG);
}

void sfml_ogl2::handle_occlusion_queries()
{
    if (!texture_atlas_)
        return;

    glCheck(glDisable(GL_TEXTURE_2D));

    frustum clip (camera_.mvp_matrix());

    const float sphere_diam = 16.f * 13.86f;

    glCheck(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
    glCheck(glDisable(GL_CULL_FACE));

    scene_.for_each_occlusion_query([&](const chunk_coordinates& pos, gl::occlusion_query& qry)
    {
		vec3f offset {vec3i{pos - chunk_offset_}};
        offset *= 256.f;

		if (clip.is_inside(offset + vec3f{128,128,128}, sphere_diam))
        {
            auto mtx = translate(camera_.model_view_matrix(), offset);
            glLoadMatrixf(mtx.as_ptr());

            switch (qry.state())
            {

            case gl::occlusion_query::inactive:
                break;

            case gl::occlusion_query::busy:
                occlusion_block_.bind();
                bind_attributes<occ_cube_vtx>();
                glColor3f(1.f, 0.f, 0.f);
                occlusion_block_.draw();
                break;

            case gl::occlusion_query::visible:
                occlusion_block_.bind();
                bind_attributes<occ_cube_vtx>();
                glColor3f(0.f, 1.f, 0.f);
                occlusion_block_.draw();
                break;

            default:
                qry.begin_query();
                occlusion_block_.bind();
                bind_attributes<occ_cube_vtx>();
                glColor3f(1.f, 1.f, 1.f);
                occlusion_block_.draw();
                qry.end_query();
            }
        }
    });

    gl::vbo::unbind();
    glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
}

void sfml_ogl2::draw(const gl::vbo& v, const matrix4<float>& mtx)
{
    if (!texture_atlas_)
        return;

    glCheck(glPushAttrib(GL_ALL_ATTRIB_BITS));
    glCheck(glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT));

    texture_atlas_.bind();
    v.bind();
    bind_attributes<ogl2_terrain_vertex>();
    v.draw();
    v.unbind();
    texture_atlas_.unbind();

    glCheck(glPopClientAttrib());
    glCheck(glPopAttrib());
}

void sfml_ogl2::draw_model(const wfpos& p, uint16_t m)
{
    (void)p; (void)m;
}

} // namespace hexa

