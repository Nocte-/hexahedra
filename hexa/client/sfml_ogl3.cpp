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

namespace hexa
{

namespace
{

static interpolated_map<float, color> sky_grad{{0.0f, {0.10f, 0.10f, 0.12f}},
                                               {0.5f, {0.56f, 0.67f, 1.0f}},
                                               {1.0f, {0.10f, 0.10f, 0.12f}}};

static interpolated_map<float, color> amb_grad{{0.0f, {0.15f, 0.15f, 0.15f}},
                                               {0.15f, {0.35f, 0.15f, 0.15f}},
                                               {0.28f, {0.56f, 0.67f, 1.0f}},
                                               {0.7f, {0.56f, 0.67f, 1.0f}},
                                               {0.8f, {0.20f, 0.20f, 0.20f}},
                                               {1.0f, {0.15f, 0.15f, 0.15f}}};

static interpolated_map<float, color> sun_grad{{0.0f, {0.0f, 0.0f, 0.0f}},
                                               {0.2f, {0.0f, 0.0f, 0.0f}},
                                               {0.3f, {1.0f, 1.0f, 0.7f}},
                                               {0.7f, {1.0f, 1.0f, 0.7f}},
                                               {0.8f, {0.0f, 0.0f, 0.0f}},
                                               {1.0f, {0.0f, 0.0f, 0.0f}}};

static interpolated_map<float, color> art_grad{{0.0f, {0.85f, 0.6f, 0.2f}},
                                               {0.29f, {0.85f, 0.6f, 0.2f}},
                                               {0.3f, {0.0f, 0.0f, 0.0f}},
                                               {0.8f, {0.0f, 0.0f, 0.0f}},
                                               {0.81f, {0.85f, 0.6f, 0.2f}},
                                               {1.0f, {0.85f, 0.6f, 0.2f}}};

chunk_index flip0(chunk_index i)
{
    return chunk_index(i.z, i.x, i.y);
}

chunk_index flip1(chunk_index i)
{
    return chunk_index(15 - i.z, 15 - i.x, i.y);
}

chunk_index flip2(chunk_index i)
{
    return chunk_index(15 - i.x, i.z, i.y);
}

chunk_index flip3(chunk_index i)
{
    return chunk_index(i.x, 15 - i.z, i.y);
}

chunk_index flip4(chunk_index i)
{
    return i;
}

chunk_index flip5(chunk_index i)
{
    return chunk_index(i.x, 15 - i.y, 15 - i.z);
}

} // anonymous namespace

class terrain_mesher_ogl3 : public terrain_mesher_i
{
public:
    terrain_mesher_ogl3(vec3i offset)
        : terrain_mesher_i{offset}
        , empty_{true}
    {
    }

    void add_face(chunk_index i, direction_type side, uint16_t texture,
                  light l)
    {
        empty_ = false;
        auto& e = rs_(i, side);
        e.texture = texture + 1;
        e.light_amb = l.ambient;
        e.light_sun = l.sunlight;
        e.light_art = l.artificial;
    }

    void add_custom_block(chunk_index i, const custom_block& model,
                          const std::vector<light>& l)
    {
        empty_ = false;
        vec3f offset{offset_ + i};

        std::array<uint8_t, 2> light;
        for (auto& part : model) {
            const auto& a(part.box.first);
            auto b(part.box.second);
            b += chunk_index(1, 1, 1);

            // Face: +x
            light[0] = l[0].artificial;
            light[1] = (l[0].ambient << 4) + l[0].sunlight;

            custom_.emplace_back(vec3f(b.x, a.y, a.z) / 16.f + offset,
                                 vector2<uint8_t>(b.y, b.z), part.textures[0],
                                 light);

            custom_.emplace_back(vec3f(b.x, b.y, a.z) / 16.f + offset,
                                 vector2<uint8_t>(a.y, b.z), part.textures[0],
                                 light);

            custom_.emplace_back(vec3f(b.x, b.y, b.z) / 16.f + offset,
                                 vector2<uint8_t>(a.y, a.z), part.textures[0],
                                 light);

            custom_.emplace_back(vec3f(b.x, a.y, b.z) / 16.f + offset,
                                 vector2<uint8_t>(b.y, a.z), part.textures[0],
                                 light);

            // Face: -x
            light[0] = l[1].artificial;
            light[1] = (l[1].ambient << 4) + l[1].sunlight;

            custom_.emplace_back(vec3f(a.x, b.y, a.z) / 16.f + offset,
                                 vector2<uint8_t>(a.y, b.z), part.textures[1],
                                 light);

            custom_.emplace_back(vec3f(a.x, a.y, a.z) / 16.f + offset,
                                 vector2<uint8_t>(b.y, b.z), part.textures[1],
                                 light);

            custom_.emplace_back(vec3f(a.x, a.y, b.z) / 16.f + offset,
                                 vector2<uint8_t>(b.y, a.z), part.textures[1],
                                 light);

            custom_.emplace_back(vec3f(a.x, b.y, b.z) / 16.f + offset,
                                 vector2<uint8_t>(a.y, a.z), part.textures[1],
                                 light);

            // Face: +y
            light[0] = l[2].artificial;
            light[1] = (l[2].ambient << 4) + l[2].sunlight;

            custom_.emplace_back(vec3f(b.x, b.y, b.z) / 16.f + offset,
                                 vector2<uint8_t>(a.x, a.z), part.textures[2],
                                 light);

            custom_.emplace_back(vec3f(b.x, b.y, a.z) / 16.f + offset,
                                 vector2<uint8_t>(a.x, b.z), part.textures[2],
                                 light);

            custom_.emplace_back(vec3f(a.x, b.y, a.z) / 16.f + offset,
                                 vector2<uint8_t>(b.x, b.z), part.textures[2],
                                 light);

            custom_.emplace_back(vec3f(a.x, b.y, b.z) / 16.f + offset,
                                 vector2<uint8_t>(b.x, a.z), part.textures[2],
                                 light);

            // Face: -y
            light[0] = l[3].artificial;
            light[1] = (l[3].ambient << 4) + l[3].sunlight;

            custom_.emplace_back(vec3f(a.x, a.y, a.z) / 16.f + offset,
                                 vector2<uint8_t>(b.x, b.z), part.textures[3],
                                 light);

            custom_.emplace_back(vec3f(b.x, a.y, a.z) / 16.f + offset,
                                 vector2<uint8_t>(a.x, b.z), part.textures[3],
                                 light);

            custom_.emplace_back(vec3f(b.x, a.y, b.z) / 16.f + offset,
                                 vector2<uint8_t>(a.x, a.z), part.textures[3],
                                 light);

            custom_.emplace_back(vec3f(a.x, a.y, b.z) / 16.f + offset,
                                 vector2<uint8_t>(b.x, a.z), part.textures[3],
                                 light);

            // Face: +z
            light[0] = l[4].artificial;
            light[1] = (l[4].ambient << 4) + l[4].sunlight;

            custom_.emplace_back(vec3f(a.x, a.y, b.z) / 16.f + offset,
                                 vector2<uint8_t>(a.x, a.y), part.textures[4],
                                 light);

            custom_.emplace_back(vec3f(b.x, a.y, b.z) / 16.f + offset,
                                 vector2<uint8_t>(b.x, a.y), part.textures[4],
                                 light);

            custom_.emplace_back(vec3f(b.x, b.y, b.z) / 16.f + offset,
                                 vector2<uint8_t>(b.x, b.y), part.textures[4],
                                 light);

            custom_.emplace_back(vec3f(a.x, b.y, b.z) / 16.f + offset,
                                 vector2<uint8_t>(a.x, b.y), part.textures[4],
                                 light);

            // Face: -z
            light[0] = l[5].artificial;
            light[1] = (l[5].ambient << 4) + l[5].sunlight;

            custom_.emplace_back(vec3f(a.x, b.y, a.z) / 16.f + offset,
                                 vector2<uint8_t>(a.x, b.y), part.textures[5],
                                 light);

            custom_.emplace_back(vec3f(b.x, b.y, a.z) / 16.f + offset,
                                 vector2<uint8_t>(b.x, b.y), part.textures[5],
                                 light);

            custom_.emplace_back(vec3f(b.x, a.y, a.z) / 16.f + offset,
                                 vector2<uint8_t>(b.x, a.y), part.textures[5],
                                 light);

            custom_.emplace_back(vec3f(a.x, a.y, a.z) / 16.f + offset,
                                 vector2<uint8_t>(a.x, a.y), part.textures[5],
                                 light);
        }
    }

    gl::vbo make_buffer() const
    {
        static const int8_t offsets[6][4][3]
            = {{{1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1}},
               {{0, 1, 1}, {0, 1, 0}, {0, 0, 0}, {0, 0, 1}},
               {{1, 1, 1}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1}},
               {{0, 0, 1}, {0, 0, 0}, {1, 0, 0}, {1, 0, 1}},
               {{0, 1, 1}, {0, 0, 1}, {1, 0, 1}, {1, 1, 1}},
               {{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}}};

        optimized_render_surface ors(optimize_greedy(rs_));
        std::vector<ogl3_terrain_vertex> data_;
        data_.swap(custom_);

        for (int dir = 0; dir < 6; ++dir) {
            std::function<chunk_index(chunk_index)> transform;
            switch (dir) {
            case 0:
                transform = flip0;
                break;
            case 1:
                transform = flip1;
                break;
            case 2:
                transform = flip2;
                break;
            case 3:
                transform = flip3;
                break;
            case 4:
                transform = flip4;
                break;
            case 5:
                transform = flip5;
                break;
            }

            const int8_t(*o)[3] = offsets[dir];

            for (auto& lyr : ors.dirs[dir]) {
                uint8_t z = lyr.first;

                for (auto j = lyr.second.begin(); j != lyr.second.end(); ++j) {
                    auto& elem = j->second;
                    if (elem.texture == 0)
                        continue;

                    auto tx = elem.texture - 1;
                    std::array<uint8_t, 2> light;
                    light[0] = elem.light_art;
                    light[1] = (elem.light_amb << 4) + elem.light_sun;

                    vec3f p = offset_ + transform(chunk_index(
                                            j->first.x,
                                            j->first.y + elem.size.y - 1, z));

                    data_.emplace_back(
                        vec3f(p.x + o[0][0], p.y + o[0][1], p.z + o[0][2]),
                        vector2<uint8_t>(0, 0), tx, light);

                    p = offset_
                        + transform(chunk_index(j->first.x, j->first.y, z));

                    data_.emplace_back(
                        vec3f(p.x + o[1][0], p.y + o[1][1], p.z + o[1][2]),
                        vector2<uint8_t>(0, 16 * elem.size.y), tx, light);

                    p = offset_
                        + transform(chunk_index(j->first.x + elem.size.x - 1,
                                                j->first.y, z));

                    data_.emplace_back(
                        vec3f(p.x + o[2][0], p.y + o[2][1], p.z + o[2][2]),
                        vector2<uint8_t>(16 * elem.size.x, 16 * elem.size.y),
                        tx, light);

                    p = offset_ + transform(chunk_index(
                                      j->first.x + elem.size.x - 1,
                                      j->first.y + elem.size.y - 1, z));

                    data_.emplace_back(
                        vec3f(p.x + o[3][0], p.y + o[3][1], p.z + o[3][2]),
                        vector2<uint8_t>(16 * elem.size.x, 0), tx, light);
                }
            }
        }

        return gl::make_vbo(data_);
    }

    bool empty() const { return empty_; }

private:
    mutable std::vector<ogl3_terrain_vertex> custom_;
    bool empty_;
    render_surface rs_;
};

//---------------------------------------------------------------------------

sfml_ogl3::sfml_ogl3(sf::RenderWindow& win, scene& s)
    : sfml{win, s}
    , textures_ready_{false}
{
    load_shader(terrain_shader_, "terrain_gl3");
    terrain_shader_.bind_attribute(0, "position");
    terrain_shader_.bind_attribute(1, "uv");
    terrain_shader_.bind_attribute(2, "texture");
    terrain_shader_.bind_attribute(3, "data");

    terrain_shader_.use();
    if (!terrain_matrix_.bind(terrain_shader_, "matrix"))
        throw std::runtime_error(
            "uniform 'matrix' not found in terrain shader");

    terrain_camera_.bind(terrain_shader_, "camera");
    tex_.bind(terrain_shader_, "tex");
    fog_color_.bind(terrain_shader_, "fog_color");
    fog_distance_.bind(terrain_shader_, "fog_distance");
    ambient_light_.bind(terrain_shader_, "amb_color");
    sunlight_.bind(terrain_shader_, "sun_color");
    artificial_light_.bind(terrain_shader_, "art_color");
    tex_ = 0;
    terrain_shader_.stop_using();

    load_shader(model_shader_, "model_gl2");
    model_shader_.bind_attribute(0, "position");
    model_shader_.bind_attribute(1, "uv");
    model_shader_.bind_attribute(2, "normal");
    model_shader_.use();
    if (!model_matrix_.bind(model_shader_, "matrix"))
        throw std::runtime_error("uniform 'matrix' not found in model shader");

    model_tex_.bind(model_shader_, "tex");
    model_tex_ = 1;
    model_shader_.stop_using();

    mrfixit_ = models("mrfixit");

    resize(width_, height_);
    sf::Mouse::setPosition(sf::Vector2i(width_ * 0.5, height_ * 0.5), app_);
}

sfml_ogl3::~sfml_ogl3()
{
}

void sfml_ogl3::load_textures(const std::vector<std::string>& name_list)
{
    textures_ready_ = false;

    for (std::string name : name_list) {
        std::string orig_name{name};
        fs::path file{resource_file(res_block_texture, name)};

        textures_.push_back(sf::Image{});

        while (!name.empty() && !fs::is_regular_file(file)) {
            auto dot = find_last(name, "_");
            if (!dot)
                break;

            name.erase(dot.begin(), name.end());
            file = resource_file(res_block_texture, name);
        }

        if (name.empty() || !fs::is_regular_file(file)) {
            std::cout << "No matching texture for " << orig_name << " or "
                      << name << std::endl;
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

renderer_i::terrain_mesher_ptr sfml_ogl3::make_terrain_mesher(vec3i offset)
{
    return std::unique_ptr<terrain_mesher_i>(new terrain_mesher_ogl3(offset));
}

void sfml_ogl3::prepare(const player& plr)
{
    sfml::prepare(plr);

    if (textures_ready_) {
        texarr_.load(textures_, 16, 16, texture::transparent);

        int slice = 0;
        for (sf::Image& img : textures_) {
            auto dim = img.getSize();
            if (dim.x == 16 && dim.y > 16) {
                gl::vbo buffer{img.getPixelsPtr(), dim.x * dim.y, 4};
                animations_.emplace_back(
                    animated_texture(slice, dim.y / 16, std::move(buffer)));
            }
            ++slice;
        }
        textures_.clear();
        textures_ready_ = false;
    }

    static int icount = 0, jcount = 1;
    if (++icount >= 40) {
        for (auto& a : animations_)
            texarr_.load(a.buffer, a.slice, (jcount % a.frame_count) * 16);

        icount = 0;
        ++jcount;
    }

    static float count = 0.5f;
    count += 0.0001f;
    if (count >= 1)
        count -= 1;

    count = 0.5f; // Eternal day

    sky_color(sky_grad(count));
    ambient_color(0.7f * color(0.6f, 0.7f, 1.0f)); // amb_grad(count));
    sun_color(0.7f * sun_grad(count));

    terrain_shader_.use();
    artificial_light_ = color(.65f, .6f, .3f); // art_grad(count);
    fog_distance_ = static_cast<float>(scene_.view_distance() * chunk_size);
    terrain_camera_ = camera_.position();
    terrain_matrix_ = camera_.mvp_matrix();
    terrain_shader_.stop_using();
}

void sfml_ogl3::opaque_pass()
{
    if (!texarr_)
        return;

    glCheck(glActiveTexture(GL_TEXTURE0));
    texarr_.bind();
    terrain_shader_.use();
    enable_attrib_array<ogl3_terrain_vertex>();

    frustum clip(camera_.mvp_matrix());
    const float sphere_diam(13.86f);

    scene_.for_each_opaque_vbo(
        [&](const chunk_coordinates& pos, const gl::vbo& vbo) {
            vec3f offset{vec3i{pos - chunk_offset_}};
            offset *= chunk_size;

            if (clip.is_inside(offset + vec3f{8, 8, 8}, sphere_diam)) {
                vbo.bind();
                bind_attributes<ogl3_terrain_vertex>();
                vbo.draw();
            }
        });

    disable_attrib_array<ogl3_terrain_vertex>();
    terrain_shader_.stop_using();
    texarr_.unbind();
    gl::vbo::unbind();
}

void sfml_ogl3::draw_model(const wfpos& p, uint16_t m)
{
    vector offset{p.relative_to(chunk_offset_ * chunk_size)};

    glCheck(glActiveTexture(GL_TEXTURE1));

    model_shader_.use();
    model_matrix_ = camera_.projection_matrix()
                    * translate(camera_.model_view_matrix(), offset);
    model_tex_ = 1;

    mrfixit_->triangles.bind();
    mrfixit_->vertices.bind();
    enable_attrib_array<model::vertex>();
    bind_attributes<model::vertex>();

    for (auto& mesh : mrfixit_->meshes) {
        mesh.tex->bind();
        mrfixit_->triangles.draw(mesh.first_triangle, mesh.nr_of_triangles);
    }

    disable_attrib_array<model::vertex>();
    mrfixit_->triangles.unbind();
    mrfixit_->vertices.unbind();
    texture::unbind();
    model_shader_.stop_using();
}

void sfml_ogl3::transparent_pass()
{
    if (!texarr_)
        return;

    frustum clip{camera_.mvp_matrix()};
    const float sphere_diam = 13.86f;

    glCheck(glDepthMask(GL_FALSE));
    glCheck(glActiveTexture(GL_TEXTURE0));
    texarr_.bind();
    terrain_shader_.use();
    enable_attrib_array<ogl3_terrain_vertex>();

    scene_.for_each_transparent_vbo(
        [&](const chunk_coordinates& pos, const gl::vbo& vbo) {
            vec3f offset{vec3i{pos - chunk_offset_}};
            offset *= chunk_size;

            if (clip.is_inside(offset + vec3f{8, 8, 8}, sphere_diam)) {
                vbo.bind();
                bind_attributes<ogl3_terrain_vertex>();
                vbo.draw();
            }
        });

    disable_attrib_array<ogl3_terrain_vertex>();
    terrain_shader_.stop_using();
    gl::vbo::unbind();
    glCheck(glDepthMask(GL_TRUE));
}

void sfml_ogl3::handle_occlusion_queries()
{
    frustum clip{camera_.mvp_matrix()};
    const float sphere_diam = 14.f;

    gl::enable(GL_BLEND);
    glCheck(glBlendFunc(GL_ZERO, GL_ONE));
    glCheck(glDisable(GL_CULL_FACE));

    occlusion_shader_.use();
    enable_attrib_array<occ_cube_vtx>();

    scene_.for_each_occlusion_query(
        [&](const chunk_coordinates& pos, gl::occlusion_query& qry) {
            vec3f offset{vec3i(pos - chunk_offset_)};
            offset *= chunk_size;

            if (clip.is_inside(offset + vec3f{8, 8, 8}, sphere_diam)) {
                auto mtx(camera_.projection_matrix()
                         * translate(camera_.model_view_matrix(), offset));
                occlusion_matrix_ = mtx;

                switch (qry.state()) {
                case gl::occlusion_query::inactive:
                    break;

                case gl::occlusion_query::busy:
                case gl::occlusion_query::visible:
                    occlusion_block_.bind();
                    bind_attributes<occ_cube_vtx>();
                    occlusion_block_.draw();
                    break;

                default:
                    qry.begin_query();
                    occlusion_block_.bind();
                    bind_attributes<occ_cube_vtx>();
                    occlusion_block_.draw();
                    qry.end_query();
                }
            }
        });

    glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    disable_attrib_array<occ_cube_vtx>();
    gl::vbo::unbind();
    occlusion_shader_.stop_using();

    glCheck(glEnable(GL_CULL_FACE));
}

void sfml_ogl3::draw(const gl::vbo& v, const matrix4<float>& mtx)
{
    if (!texarr_)
        return;

    glCheck(glEnable(GL_TEXTURE_2D));
    glCheck(glActiveTexture(GL_TEXTURE0));
    texarr_.bind();
    terrain_shader_.use();
    terrain_matrix_ = mtx;
    terrain_camera_ = vec3f{0, 0, 0};

    enable_attrib_array<ogl3_terrain_vertex>();
    v.bind();
    bind_attributes<ogl3_terrain_vertex>();
    v.draw();
    v.unbind();
    disable_attrib_array<ogl3_terrain_vertex>();
    terrain_shader_.stop_using();
    texarr_.unbind();
}

} // namespace hexa
