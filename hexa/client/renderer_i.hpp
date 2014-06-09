//---------------------------------------------------------------------------
/// \file   client/renderer_i.hpp
/// \brief  Interface for 3D rendering backend.
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

#include <list>
#include <memory>
#include <string>

#include <boost/signals2.hpp>

#include <hexa/basic_types.hpp>
#include <hexa/color.hpp>
#include <hexa/matrix.hpp>
#include <hexa/wfpos.hpp>

#include "scene.hpp"
#include "terrain_mesher_i.hpp"
#include "texture.hpp"
#include "types.hpp"
#include "occlusion_manager.hpp"

namespace hexa
{

class game;
class scene;
class player;
class hud;
struct event;

/** Interface for rendering backends. */
class renderer_i
{
public:
    typedef std::unique_ptr<terrain_mesher_i> terrain_mesher_ptr;

public:
    renderer_i(scene& s)
        : scene_{s}
        , chunk_offset_{world_chunk_center}
    {
    }

    virtual ~renderer_i() {}

    virtual void load_textures(const std::vector<std::string>& textures) = 0;
    virtual void prepare(const player& plr) = 0;
    virtual void opaque_pass() = 0;
    virtual void handle_occlusion_queries() = 0;
    virtual void transparent_pass() = 0;
    virtual void draw_ui(double elapsed_time, const hud& info) = 0;
    virtual void display() = 0;
    virtual void resize(unsigned int w, unsigned int h) = 0;

    virtual void sky_color(const color&) = 0;
    virtual void ambient_color(const color&) = 0;
    virtual void sun_color(const color&) = 0;

    virtual terrain_mesher_ptr make_terrain_mesher(vec3i offset)
    {
        return nullptr;
    }

    virtual void draw(const gl::vbo& v, const matrix4<float>& mtx) = 0;
    virtual void draw_model(const wfpos& p, uint16_t m) = 0;

    virtual void offset(chunk_coordinates pos) { chunk_offset_ = pos; }

    chunk_coordinates offset() const { return chunk_offset_; }

    virtual void process(const event& ev) {}
    virtual void process(const sf::Event& ev) {}

    virtual void highlight_face(const pos_dir<world_coordinates>& face,
                                const color_alpha& hl_color)
    {
    }

    virtual void highlight_block(world_coordinates block,
                                 const color_alpha& hl_color)
    {
    }

    virtual void highlight_custom_block(world_coordinates block,
                                        const custom_block& model,
                                        const color_alpha& hl_color)
    {
    }

protected:
    scene& scene_;
    chunk_coordinates chunk_offset_;
};

} // namespace hexa
