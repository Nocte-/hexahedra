//---------------------------------------------------------------------------
// client/sfml_ogl3.cpp
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

#include "sfml_ogl3.hpp"

#include <atomic>

#include <boost/range/algorithm.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/thread/locks.hpp>
#include <boost/algorithm/string.hpp>

#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>

#include <hexa/basic_types.hpp>
#include <hexa/block_types.hpp>
#include <hexa/voxel_algorithm.hpp>
#include <hexa/voxel_range.hpp>
#include <hexa/interpolated_map.hpp>
#include <hexa/frustum.hpp>

#include "game.hpp"
#include "player.hpp"
#include "scene.hpp"
#include "texture.hpp"
#include "opengl.hpp"
#include "opengl_vertex.hpp"
#include "shader.hpp"
#include "sky_shader.hpp"
#include "sfml_resource_manager.hpp"
#include "render_surface.hpp"

using namespace boost;
using namespace boost::range;
using namespace boost::adaptors;
using namespace boost::numeric::ublas;

namespace fs = boost::filesystem;

namespace hexa {

namespace {

static interpolated_map<float, color> sky_grad
    { { 0.0f , { 0.10f, 0.10f, 0.12f } },
      { 0.5f , { 0.56f, 0.67f, 1.0f } },
      { 1.0f , { 0.10f, 0.10f, 0.12f } } };

static interpolated_map<float, color> amb_grad
    { { 0.0f , { 0.2f, 0.2f, 0.2f } },
      { 0.2f , { 0.2f, 0.2f, 0.2f } },
      { 0.3f , { 0.3f, 0.1f, 0.1f } },
      { 0.4f , { 0.56f, 0.67f, 1.0f } },
      { 0.6f , { 0.56f, 0.67f, 1.0f } },
      { 0.8f , { 0.2f, 0.2f, 0.2f } },
      { 1.0f , { 0.2f, 0.2f, 0.2f } } };

static interpolated_map<float, color> sun_grad
    { { 0.0f , { 0.0f, 0.0f, 0.0f } },
      { 0.2f , { 0.0f, 0.0f, 0.0f } },
      { 0.3f , { 1.0f, 1.0f, 0.7f } },
      { 0.6f , { 1.0f, 1.0f, 0.7f } },
      { 0.7f , { 0.0f, 0.0f, 0.0f } },
      { 1.0f , { 0.0f, 0.0f, 0.0f } } };

static interpolated_map<float, color> art_grad
    { { 0.0f , { 0.85f, 0.6f, 0.2f } },
      { 0.29f, { 0.85f, 0.6f, 0.2f } },
      { 0.3f , { 0.0f, 0.0f, 0.0f } },
      { 0.8f , { 0.0f, 0.0f, 0.0f } },
      { 0.81f, { 0.85f, 0.6f, 0.2f } },
      { 1.0f , { 0.85f, 0.6f, 0.2f } } };


chunk_index flip0 (chunk_index i)
    { return chunk_index(i.z, i.x, i.y); }

chunk_index flip1 (chunk_index i)
    { return chunk_index(15-i.z, 15-i.x, i.y); }

chunk_index flip2 (chunk_index i)
    { return chunk_index(15-i.x, i.z, i.y); }

chunk_index flip3 (chunk_index i)
    { return chunk_index(i.x, 15-i.z, i.y); }

chunk_index flip4 (chunk_index i)
    { return i; }

chunk_index flip5 (chunk_index i)
    { return chunk_index(i.x, 15-i.y, 15-i.z); }


} // anonymous namespace

typedef vertex_1<vtx_xyz<int16_t> > occ_cube_vtx;

class terrain_mesher_ogl3 : public terrain_mesher_i
{
public:
    terrain_mesher_ogl3()
        : empty_(true)
    {
    }

    void add_face(chunk_index i, direction_type side,
                  uint16_t texture, light l)
    {
        empty_ = false;

        auto& e (rs_(i, side));
        e.texture = texture + 1;
        e.light_amb = l.ambient;
        e.light_sun = l.sunlight;
        e.light_art = l.artificial;
    }

    void add_custom_block (chunk_index i, const custom_block& model, const std::vector<light>& l)
    {
        empty_ = false;

        vector3<uint16_t> offset (i);
        offset *= chunk_size;
        std::array<uint8_t, 2> light;
        for (auto& part : model)
        {
            const auto& a (part.box.first);
            auto b (part.box.second);
            b += chunk_index(1,1,1);

            // Face: +x
            light[0] = l[0].artificial;
            light[1] = (l[0].ambient << 4) + l[0].sunlight;

            custom_.emplace_back(vector3<uint16_t>(b.x, a.y, a.z) + offset,
                                 vector2<uint8_t>(b.y, b.z), part.textures[0], light);

            custom_.emplace_back(vector3<uint16_t>(b.x, b.y, a.z) + offset,
                                 vector2<uint8_t>(a.y, b.z), part.textures[0], light);

            custom_.emplace_back(vector3<uint16_t>(b.x, b.y, b.z) + offset,
                                 vector2<uint8_t>(a.y, a.z), part.textures[0], light);

            custom_.emplace_back(vector3<uint16_t>(b.x, a.y, b.z) + offset,
                                 vector2<uint8_t>(b.y, a.z), part.textures[0], light);

            // Face: -x
            light[0] = l[1].artificial;
            light[1] = (l[1].ambient << 4) + l[1].sunlight;

            custom_.emplace_back(vector3<uint16_t>(a.x, b.y, a.z) + offset,
                                 vector2<uint8_t>(a.y, b.z), part.textures[1], light);

            custom_.emplace_back(vector3<uint16_t>(a.x, a.y, a.z) + offset,
                                 vector2<uint8_t>(b.y, b.z), part.textures[1], light);

            custom_.emplace_back(vector3<uint16_t>(a.x, a.y, b.z) + offset,
                                 vector2<uint8_t>(b.y, a.z), part.textures[1], light);

            custom_.emplace_back(vector3<uint16_t>(a.x, b.y, b.z) + offset,
                                 vector2<uint8_t>(a.y, a.z), part.textures[1], light);

            // Face: +y
            light[0] = l[2].artificial;
            light[1] = (l[2].ambient << 4) + l[2].sunlight;

            custom_.emplace_back(vector3<uint16_t>(b.x, b.y, b.z) + offset,
                                 vector2<uint8_t>(a.x, a.z), part.textures[2], light);

            custom_.emplace_back(vector3<uint16_t>(b.x, b.y, a.z) + offset,
                                 vector2<uint8_t>(a.x, b.z), part.textures[2], light);

            custom_.emplace_back(vector3<uint16_t>(a.x, b.y, a.z) + offset,
                                 vector2<uint8_t>(b.x, b.z), part.textures[2], light);

            custom_.emplace_back(vector3<uint16_t>(a.x, b.y, b.z) + offset,
                                 vector2<uint8_t>(b.x, a.z), part.textures[2], light);

            // Face: -y
            light[0] = l[3].artificial;
            light[1] = (l[3].ambient << 4) + l[3].sunlight;

            custom_.emplace_back(vector3<uint16_t>(a.x, a.y, a.z) + offset,
                                 vector2<uint8_t>(b.x, b.z), part.textures[3], light);

            custom_.emplace_back(vector3<uint16_t>(b.x, a.y, a.z) + offset,
                                 vector2<uint8_t>(a.x, b.z), part.textures[3], light);

            custom_.emplace_back(vector3<uint16_t>(b.x, a.y, b.z) + offset,
                                 vector2<uint8_t>(a.x, a.z), part.textures[3], light);

            custom_.emplace_back(vector3<uint16_t>(a.x, a.y, b.z) + offset,
                                 vector2<uint8_t>(b.x, a.z), part.textures[3], light);

            // Face: +z
            light[0] = l[4].artificial;
            light[1] = (l[4].ambient << 4) + l[4].sunlight;

            custom_.emplace_back(vector3<uint16_t>(a.x, a.y, b.z) + offset,
                                 vector2<uint8_t>(a.x, a.y), part.textures[4], light);

            custom_.emplace_back(vector3<uint16_t>(b.x, a.y, b.z) + offset,
                                 vector2<uint8_t>(b.x, a.y), part.textures[4], light);

            custom_.emplace_back(vector3<uint16_t>(b.x, b.y, b.z) + offset,
                                 vector2<uint8_t>(b.x, b.y), part.textures[4], light);

            custom_.emplace_back(vector3<uint16_t>(a.x, b.y, b.z) + offset,
                                 vector2<uint8_t>(a.x, b.y), part.textures[4], light);

            // Face: -z
            light[0] = l[5].artificial;
            light[1] = (l[5].ambient << 4) + l[5].sunlight;

            custom_.emplace_back(vector3<uint16_t>(a.x, b.y, a.z) + offset,
                                 vector2<uint8_t>(a.x, b.y), part.textures[5], light);

            custom_.emplace_back(vector3<uint16_t>(b.x, b.y, a.z) + offset,
                                 vector2<uint8_t>(b.x, b.y), part.textures[5], light);

            custom_.emplace_back(vector3<uint16_t>(b.x, a.y, a.z) + offset,
                                 vector2<uint8_t>(b.x, a.y), part.textures[5], light);

            custom_.emplace_back(vector3<uint16_t>(a.x, a.y, a.z) + offset,
                                 vector2<uint8_t>(a.x, a.y), part.textures[5], light);

        }
    }

    gl::vbo make_buffer() const
    {
        static const int8_t offsets[6][4][3] =
            { { {1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1} },
              { {0, 1, 1}, {0, 1, 0}, {0, 0, 0}, {0, 0, 1} },
              { {1, 1, 1}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1} },
              { {0, 0, 1}, {0, 0, 0}, {1, 0, 0}, {1, 0, 1} },
              { {0, 1, 1}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1} },
              { {0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0} } };

        optimized_render_surface ors (optimize_greedy(rs_));
        std::vector<ogl3_terrain_vertex> data_;
        data_.swap(custom_);

        for (int dir(0); dir < 6; ++dir)
        {
            std::function<chunk_index(chunk_index)> transform;
            switch (dir)
            {
                case 0 : transform = flip0; break;
                case 1 : transform = flip1; break;
                case 2 : transform = flip2; break;
                case 3 : transform = flip3; break;
                case 4 : transform = flip4; break;
                case 5 : transform = flip5; break;
            }

            const int8_t(*o)[3] = offsets[dir];

            for (auto& lyr : ors.dirs[dir])
            {
                uint8_t z (lyr.first);

                for (auto j (lyr.second.begin()); j != lyr.second.end(); ++j)
                {
                    auto& elem (j->second);
                    if (elem.texture == 0)
                        continue;

                    auto tx (elem.texture - 1);
                    std::array<uint8_t, 2> light;
                    light[0] = elem.light_art;
                    light[1] = (elem.light_amb << 4) + elem.light_sun;

                    chunk_index p;
                    p = transform(chunk_index(j->first.x,j->first.y + elem.size.y - 1,z));

                    data_.emplace_back(
                        vector3<uint16_t>(p.x+o[0][0], p.y+o[0][1], p.z+o[0][2]) * 16,
                        vector2<uint8_t>(0, 0), tx, light);

                    p = transform(chunk_index(j->first.x, j->first.y,z));

                    data_.emplace_back(
                        vector3<uint16_t>(p.x+o[1][0], p.y+o[1][1], p.z+o[1][2]) * 16,
                        vector2<uint8_t>(0, 16*elem.size.y), tx, light);

                    p = transform(chunk_index(j->first.x + elem.size.x - 1,j->first.y,z));

                    data_.emplace_back(
                        vector3<uint16_t>(p.x+o[2][0], p.y+o[2][1], p.z+o[2][2]) * 16,
                        vector2<uint8_t>(16 * elem.size.x, 16 * elem.size.y), tx, light);

                    p = transform(chunk_index(j->first.x + elem.size.x - 1,j->first.y + elem.size.y - 1,z));

                    data_.emplace_back(
                        vector3<uint16_t>(p.x+o[3][0], p.y+o[3][1], p.z+o[3][2]) * 16,
                        vector2<uint8_t>(16 * elem.size.x, 0), tx, light);
                }
            }
        }

        return gl::make_vbo(data_);
    }

    bool empty() const
    {
        return empty_;
    }

private:
    mutable std::vector<ogl3_terrain_vertex> custom_;
    bool            empty_;
    render_surface  rs_;
};



//---------------------------------------------------------------------------

sfml_ogl3::sfml_ogl3(sf::RenderWindow& win)
    : sfml(win)
    , textures_ready_(false)
{
    terrain_shader_.load(resource_file(res_shader, "terrain_gl3"));

    terrain_shader_.bind_attribute(0, "position");
    terrain_shader_.bind_attribute(1, "uv");
    terrain_shader_.bind_attribute(2, "texture");
    terrain_shader_.bind_attribute(3, "data");

    if (!terrain_shader_.link())
    {
        std::cerr << "Could not link GLSL!" << std::endl;
        std::cout << terrain_shader_.info_log() << std::endl;
    }

    terrain_shader_.use();
    tex_.bind(terrain_shader_, "tex");
    fog_color_.bind(terrain_shader_, "fog_color");
    fog_density_.bind(terrain_shader_, "fog_density");
    ambient_light_.bind(terrain_shader_, "amb_color");
    sunlight_.bind(terrain_shader_, "sun_color");
    artificial_light_.bind(terrain_shader_, "art_color");
    tex_ = 0;
    terrain_shader_.stop_using();

    // Note: using gl2 for the time being.
    model_shader_.load(resource_file(res_shader, "model_gl2"));
    if (!model_shader_.link())
    {
        std::cerr << "Could not link model shader" << std::endl;
        std::cout << model_shader_.info_log() << std::endl;
        throw(0);
    }

    mrfixit_ = models("mrfixit");

    resize(width_, height_);
    sf::Mouse::setPosition(sf::Vector2i(width_ * 0.5, height_ * 0.5), app_);

    const int16_t l (-10), h (266);

    static const int16_t cube[24*3] = {
        l,l,l,  h,l,l,  h,h,l,  l,h,l,
        h,l,l,  h,l,h,  h,h,h,  h,h,l,
        l,h,h,  h,h,h,  h,l,h,  l,l,h,
        l,h,l,  l,h,h,  l,l,h,  l,l,l,
        l,l,l,  h,l,l,  h,l,h,  l,l,h,
        l,h,h,  h,h,h,  h,h,l,  l,h,l };

    occlusion_block_ = gl::vbo(cube, 24, sizeof(occ_cube_vtx));
}

sfml_ogl3::~sfml_ogl3()
{
}

void sfml_ogl3::load_textures(const std::vector<std::string>& name_list)
{
    textures_ready_ = false;

    for (std::string name : name_list)
    {
        std::string orig_name (name);
        fs::path file (resource_file(res_block_texture, name));

        textures_.push_back(sf::Image());

        while (!name.empty() && !fs::is_regular_file(file))
        {
            auto dot (find_last(name, "."));
            if (!dot)
                break;

            name.erase(dot.begin(), name.end());
            file = resource_file(res_block_texture, name);
        }

        if (name.empty() || !fs::is_regular_file(file))
        {
            std::cout << "No matching texture for " << orig_name << " or " << name << std::endl;
            file = resource_file(res_block_texture, "unknown");
        }
        textures_.back().loadFromFile(file.string());
    }

    textures_ready_ = true;
}

void sfml_ogl3::sky_color(const color& c)
{
    glClearColor(c.r(), c.g(), c.b(), 1.0);
}

void sfml_ogl3::sun_color(const color& rgb)
{
    terrain_shader_.use();
    sunlight_ = rgb;
    terrain_shader_.stop_using();
}

void sfml_ogl3::ambient_color(const color& rgb)
{
    terrain_shader_.use();
    fog_color_ = horizon_color_;
    ambient_light_ = rgb;
    terrain_shader_.stop_using();
}

std::unique_ptr<terrain_mesher_i>
sfml_ogl3::make_terrain_mesher()
{
    return std::unique_ptr<terrain_mesher_i>(new terrain_mesher_ogl3);
}

void sfml_ogl3::prepare(const player& plr)
{
    if (textures_ready_)
    {
        texarr_.load(textures_, 16, 16, texture::transparent);
        textures_.clear();
        textures_ready_ = false;
    }

    static float count (0.5);
    count += 0.0001f;
    if (count >= 1)
        count -= 1;

    //count = 0.5;

    sky_color(sky_grad(count));
    ambient_color(amb_grad(count));
    sun_color(sun_grad(count));
    terrain_shader_.use();
    artificial_light_ = color(1,1,.8f); // art_grad(count);
    fog_density_ = 2.2f / (float)(view_dist_ * chunk_size * 6);
    terrain_shader_.stop_using();

    sfml::prepare(plr);
    move_player(plr.chunk_position());

    for(chunk_coordinates c : process_vbo_queue())
        on_new_vbo(c);
}

void sfml_ogl3::opaque_pass()
{
    if (!texarr_)
        return;

    boost::mutex::scoped_lock l (lock);

    glCheck(glActiveTexture(GL_TEXTURE0));
    texarr_.bind();
    terrain_shader_.use();
    enable_vertex_attributes<ogl3_terrain_vertex>();

    frustum clip (camera_.mvp_matrix());

    const float sphere_diam (16.f * 13.86f);
    for (const auto& l : opaque_vbos)
    {
        for (const auto& v : l)
        {
            assert(v.second.id() != 0);
            assert(v.second.vertex_count() != 0);

            vector3<float> offset (vector3<int>(v.first * chunk_size - world_offset_));
            offset *= 16.f;

            if (v.second.id() && clip.is_inside(vector3<float>(offset.x + 128, offset.y + 128, offset.z + 128), sphere_diam))
            {
                auto mtx (translate(camera_.model_view_matrix(), offset));
                glCheck(glLoadMatrixf(mtx.as_ptr()));
                v.second.bind();
                bind_attributes<ogl3_terrain_vertex>();
                v.second.draw();
            }
        }
    }

    disable_vertex_attributes<ogl3_terrain_vertex>();
    terrain_shader_.stop_using();
    texarr_.unbind();
    gl::vbo::unbind();
}

void sfml_ogl3::draw_model(const wfpos& p, uint16_t m) const
{
    static  GLfloat zero[4] = { 0, 0, 0, 0 },
                         one[4] = { 1, 1, 1, 1 },
                         ambientcol[4] = { 0.5f, 0.5f, 0.5f, 1 },
                         diffusecol[4] = { 1.0f, 1.0f, 1.0f, 1 },
                         lightdir[4] = { 0, 0, 1, 0 };

    glCheck(glLightModelfv(GL_LIGHT_MODEL_AMBIENT, zero));
    glCheck(glMaterialfv(GL_FRONT, GL_SPECULAR, zero));
    glCheck(glMaterialfv(GL_FRONT, GL_EMISSION, zero));
    glCheck(glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, one));
    glCheck(glLightfv(GL_LIGHT0, GL_SPECULAR, zero));
    glCheck(glLightfv(GL_LIGHT0, GL_AMBIENT, ambientcol));
    glCheck(glLightfv(GL_LIGHT0, GL_DIFFUSE, diffusecol));
    glCheck(glLightfv(GL_LIGHT0, GL_POSITION, lightdir));

    vector offset (p.relative_to(world_offset_));
    //trace("%1%", p);
    //trace("%1% -> %2%", world_offset_, offset);

    //std::cout << "draw model at " << p.pos << " => " << offset << std::endl;
    offset *= 16.f;

    //gl::guard_matrix();
    glCheck(glMatrixMode(GL_MODELVIEW));
    auto mtx (translate(camera_.model_view_matrix(), offset));
    glCheck(glLoadMatrixf(mtx.as_ptr()));
    glCheck(glColor3f(1, 1, 1));
    mrfixit_->triangles.bind();
    mrfixit_->vertices.bind();

    hexa::bind_attributes_ogl2<model::vertex>();

    gl::enable ({GL_LIGHTING, GL_LIGHT0, GL_NORMALIZE, GL_TEXTURE_2D});
    gl::client_states ecs (hexa::client_states<model::vertex>());

    for (auto& mesh : mrfixit_->meshes)
    {
        mesh.tex->bind();
        //glBindTexture(GL_TEXTURE_2D, mesh.tex.id());
        mrfixit_->triangles.draw(mesh.first_triangle, mesh.nr_of_triangles);
    }
    gl::disable ({ GL_LIGHTING, GL_LIGHT0 });

    mrfixit_->triangles.unbind();
    mrfixit_->vertices.unbind();
}

void sfml_ogl3::transparent_pass()
{
    if (!texarr_)
        return;

    boost::mutex::scoped_lock l (lock);

    enable_vertex_attributes<ogl3_terrain_vertex>();
    frustum clip (camera_.mvp_matrix());

    glCheck(glDepthMask(GL_FALSE));
    glCheck(glActiveTexture(GL_TEXTURE0));
    texarr_.bind();
    terrain_shader_.use();
    const float sphere_diam (16.f * 13.86f);

    for (const auto& l : transparent_vbos | reversed)
    {
        for (const auto& v : l)
        {
            assert(v.second.id() != 0);
            assert(v.second.vertex_count() != 0);

            vector3<float> offset (vector3<int>(v.first * chunk_size - world_offset_));
            offset *= 16.f;

            if (v.second.id() && clip.is_inside(vector3<float>(offset.x + 128.f, offset.y + 128.f, offset.z + 128.f), sphere_diam))
            {
                auto mtx (translate(camera_.model_view_matrix(), offset));
                glCheck(glLoadMatrixf(mtx.as_ptr()));

                v.second.bind();
                bind_attributes<ogl3_terrain_vertex>();
                v.second.draw();
            }
        }
    }

    glCheck(glLoadMatrixf(camera_.model_view_matrix().as_ptr()));
    disable_vertex_attributes<ogl3_terrain_vertex>();
    terrain_shader_.stop_using();
    gl::vbo::unbind();
    glCheck(glDepthMask(GL_TRUE));
}


bool debug_mode = false;

void sfml_ogl3::handle_occlusion_queries()
{
    boost::mutex::scoped_lock l (lock);

    glCheck(glLoadMatrixf(camera_.model_view_matrix().as_ptr()));
    glCheck(glDisable(GL_TEXTURE_2D));

    frustum clip (camera_.mvp_matrix());

    const float sphere_diam (16.f * 13.86f);

    if (!debug_mode)
        glCheck(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));

    glCheck(glDisable(GL_CULL_FACE));
    enable_vertex_attributes<occ_cube_vtx>();

	for (oqs_t& l : occlusion_queries)
    {
		for (sfml_ogl3::oqs_t::value_type& v : l)
        {
            vector3<float> offset (vector3<int>(v.first * chunk_size - world_offset_));
            offset *= 16.f;

            if (clip.is_inside(vector3<float>(offset.x + 128, offset.y + 128, offset.z + 128), sphere_diam))
            {
                occlusion_query& qry (v.second);
                glTranslatef(offset.x, offset.y, offset.z);

                if (qry.state() == occlusion_query::busy)
                {
                    occlusion_block_.bind();
                    bind_attributes<occ_cube_vtx>();
                    glColor3f(1.f, 0.f, 0.f);
                    occlusion_block_.draw();
                }
                else if (qry.state() == occlusion_query::visible)
                {
                    occlusion_block_.bind();
                    bind_attributes<occ_cube_vtx>();
                    glColor3f(0.f, 1.f, 0.f);
                    occlusion_block_.draw();
                }
                else if (qry.state() == occlusion_query::air)
                {
                    occlusion_block_.bind();
                    bind_attributes<occ_cube_vtx>();
                    glColor3f(0.5f, 0.5f, 0.5f);
                    occlusion_block_.draw();
                }
                else
                {
                    qry.begin_query();
                    occlusion_block_.bind();
                    bind_attributes<occ_cube_vtx>();
                    glColor3f(1.f, 1.f, 1.f);
                    occlusion_block_.draw();
                    qry.end_query();
                }
                glTranslatef(-offset.x, -offset.y, -offset.z);
            }
        }
    }

    gl::vbo::unbind();
    disable_vertex_attributes<occ_cube_vtx>();
    glCheck(glEnable(GL_CULL_FACE));
    glCheck(glEnable(GL_TEXTURE_2D));
    glCheck(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
}

void sfml_ogl3::draw(const gl::vbo& v) const
{
    glCheck(glPushAttrib(GL_ALL_ATTRIB_BITS));
    glCheck(glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT));
    glCheck(glEnable(GL_TEXTURE_2D));
    glCheck(glEnable(GL_CULL_FACE));

    texarr_.bind();
    terrain_shader_.use();
    enable_vertex_attributes<ogl3_terrain_vertex>();

    v.bind();
    bind_attributes<ogl3_terrain_vertex>();
    v.draw();
    v.unbind();

    terrain_shader_.stop_using();
    texarr_.unbind();
    glCheck(glPopClientAttrib());
    glCheck(glPopAttrib());
}

} // namespace hexa

