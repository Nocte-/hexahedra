//---------------------------------------------------------------------------
/// \file   client/sfml.hpp
/// \brief  Render backend and event handling based on SFML.
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
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <array>
#include <list>
#include <string>
#include <tuple>
#include <unordered_map>

#include <boost/thread/mutex.hpp>

#include <GL/glew.h>
#include <GL/gl.h>
#include <SFML/Window.hpp>
/*
#include <Gwen/Gwen.h>
#include <Gwen/Renderers/SFML2.h>
#include <Gwen/Input/SFML.h>
#include <Gwen/Skins/TexturedBase.h>
*/
#include <hexa/basic_types.hpp>
#include <hexa/matrix.hpp>

#include "camera.hpp"
#include "occlusion_query.hpp"
#include "opengl_vertex.hpp"
#include "player.hpp"
#include "renderer_chunk_management.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "visibility_test.hpp"

namespace hexa {

class game;
class scene;
class hud;

class sfml : public renderer_chunk_management
{
public:
    /** Constructor. */
    sfml(sf::RenderWindow& win);
    virtual ~sfml();

    virtual void prepare(const player& plr);
    void draw_ui(double elapsed_time, const hud& info);
    void display();
    visibility_tests get_finished_queries();
    virtual void resize(unsigned int w, unsigned int h);
    void waiting_screen() const;
    void process (const event& ev);
    void process (const sf::Event& ev);

protected:
    void draw_chunk_cube(const chunk_coordinates& pos);
    void draw_chunk_face(const chunk_coordinates& pos, direction_type dir);

    struct query : public occlusion_query, public visibility_test
    {
        query(const chunk_coordinates& pos, direction_type d)
            : visibility_test(pos, d, pending_query)
        { }

        query(const visibility_test& t) : visibility_test (t) {}
    };

    void draw_bar(float x, float y, int index, int width, double ratio);

    void draw_hotbar (const hud &h);

protected:
    camera          camera_;
    wfpos           camera_wpos_;
    color           horizon_color_;

    int     width_;
    int     height_;

    sf::RenderWindow& app_;

    std::list<query>    waiting_queries;
    std::list<query>    active_queries;

    std::array<sf::Sprite, 256>  ui_elem_;

    std::shared_ptr<sf::Texture> ui_img_;
    texture sun_;
    texture moon_;
    texture star_;

    std::shared_ptr<sf::Font> ui_font_;

    sf::RenderTexture hotbar_;


//    Gwen::Renderer::SFML2   GwenRenderer;
//    Gwen::Input::SFML       GwenInput;
//    Gwen::Skin::TexturedBase Skin;
//    Gwen::Controls::Canvas* pCanvas;

};

} // namespace hexa

