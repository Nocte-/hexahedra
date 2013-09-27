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
// Copyright 2012-2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <memory>
#include <string>

#include <boost/signals2.hpp>

#include <hexa/basic_types.hpp>
#include <hexa/color.hpp>
#include <hexa/wfpos.hpp>

#include "scene.hpp"
#include "terrain_mesher_i.hpp"
#include "texture.hpp"
#include "types.hpp"
#include "occlusion_manager.hpp"

namespace hexa {

class game;
class player;
class hud;
struct event;

/** Interface for rendering backends. */
class renderer_i
{
public:
    boost::signals2::signal<void (chunk_coordinates)> on_new_vbo;

    occlusion_manager  occ_mgr;

    typedef std::unique_ptr<terrain_mesher_i> terrain_mesher_ptr;

public:
    virtual ~renderer_i() {}

    virtual void view_distance(size_t distance) = 0;
    virtual void load_textures(const std::vector<std::string>& textures) = 0;
    virtual void remove_chunks(const std::vector<chunk_coordinates>& list) = 0;
    virtual void prepare(const player& plr) = 0;
    virtual void opaque_pass() = 0;
    virtual void handle_occlusion_queries() = 0;
    virtual void transparent_pass() = 0;
    virtual void draw_ui(double elapsed_time, const hud& info) = 0;
    virtual void display() = 0;
    virtual void resize(unsigned int w, unsigned int h) = 0;

    virtual void add_occlusion_query(chunk_coordinates pos) = 0;
    virtual void cancel_occlusion_query(chunk_coordinates pos) = 0;

    virtual std::deque<chunk_coordinates>
                 get_visible_queries() = 0;

    virtual void sky_color(const color&) = 0;
    virtual void ambient_color(const color&) = 0;
    virtual void sun_color(const color&) = 0;

    virtual void on_update_height(map_coordinates pos, chunk_height z) = 0;

    virtual terrain_mesher_ptr make_terrain_mesher() { return nullptr; }

    virtual void queue_meshes(chunk_coordinates pos,
                              terrain_mesher_ptr opaque,
                              terrain_mesher_ptr transparent) = 0;

    virtual void draw(const gl::vbo& v) const = 0;
    virtual void draw_model(const wfpos& p, uint16_t m) const = 0;

    virtual void set_offset (world_coordinates pos)
        { world_offset_ = pos; }

    virtual void waiting_screen() const = 0;

    virtual void process (const event& ev) { }
    virtual void process (const sf::Event& ev) { }

protected:
    world_coordinates world_offset_;
};

} // namespace hexa

