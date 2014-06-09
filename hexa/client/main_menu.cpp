//---------------------------------------------------------------------------
// client/main_menu.cpp
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
// Copyright 2012, 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "main_menu.hpp"

#include <array>
#include <cstdlib>
#include <iostream>

#include <boost/program_options/variables_map.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Utf.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "../algorithm.hpp"
#include "main_game.hpp"
#include "opengl.hpp"
#include "opengl_vertex.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "vbo.hpp"
#include "edge_buffer.hpp"
#include "event.hpp"

namespace po = boost::program_options;

namespace hexa
{

extern po::variables_map global_settings;

static GLuint cubes;

//---------------------------------------------------------------------------

namespace
{

double random()
{
    return (double)std::rand() / (double)RAND_MAX;
}

void setup_dl()
{
    const double cb(0.12), cv(0.026);

    cubes = glGenLists(1);
    glNewList(cubes, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for (int i(0); i < 2000; ++i) {
        double a(random() * 3.14159 * 2.);
        double d(random() * 50. + 3.);

        float x(2 * std::floor(std::sin(a) * d)),
            y(2 * std::floor(std::cos(a) * d)),
            z(2 * std::floor(random() * 100. - 50.));

        glColor4f(cb - random() * cv, cb - random() * cv, cb - random() * cv,
                  1);

        // front faces
        glNormal3f(0, 0, 1);
        // face v0-v1-v2
        glVertex3f(1 + x, 1 + y, 1 + z);
        glVertex3f(-1 + x, 1 + y, 1 + z);
        glVertex3f(-1 + x, -1 + y, 1 + z);
        // face v2-v3-v0
        glVertex3f(-1 + x, -1 + y, 1 + z);
        glVertex3f(1 + x, -1 + y, 1 + z);
        glVertex3f(1 + x, 1 + y, 1 + z);

        // right faces
        glNormal3f(1, 0, 0);
        // face v0-v3-v4
        glVertex3f(1 + x, 1 + y, 1 + z);
        glVertex3f(1 + x, -1 + y, 1 + z);
        glVertex3f(1 + x, -1 + y, -1 + z);
        // face v4-v5-v0
        glVertex3f(1 + x, -1 + y, -1 + z);
        glVertex3f(1 + x, 1 + y, -1 + z);
        glVertex3f(1 + x, 1 + y, 1 + z);

        // top faces
        glNormal3f(0, 1, 0);
        // face v0-v5-v6
        glVertex3f(1 + x, 1 + y, 1 + z);
        glVertex3f(1 + x, 1 + y, -1 + z);
        glVertex3f(-1 + x, 1 + y, -1 + z);
        // face v6-v1-v0
        glVertex3f(-1 + x, 1 + y, -1 + z);
        glVertex3f(-1 + x, 1 + y, 1 + z);
        glVertex3f(1 + x, 1 + y, 1 + z);

        // left faces
        glNormal3f(-1, 0, 0);
        // face  v1-v6-v7
        glVertex3f(-1 + x, 1 + y, 1 + z);
        glVertex3f(-1 + x, 1 + y, -1 + z);
        glVertex3f(-1 + x, -1 + y, -1 + z);
        // face v7-v2-v1
        glVertex3f(-1 + x, -1 + y, -1 + z);
        glVertex3f(-1 + x, -1 + y, 1 + z);
        glVertex3f(-1 + x, 1 + y, 1 + z);

        // bottom faces
        glNormal3f(0, -1, 0);
        // face v7-v4-v3
        glVertex3f(-1 + x, -1 + y, -1 + z);
        glVertex3f(1 + x, -1 + y, -1 + z);
        glVertex3f(1 + x, -1 + y, 1 + z);
        // face v3-v2-v7
        glVertex3f(1 + x, -1 + y, 1 + z);
        glVertex3f(-1 + x, -1 + y, 1 + z);
        glVertex3f(-1 + x, -1 + y, -1 + z);

        // back faces
        glNormal3f(0, 0, -1);
        // face v4-v7-v6
        glVertex3f(1 + x, -1 + y, -1 + z);
        glVertex3f(-1 + x, -1 + y, -1 + z);
        glVertex3f(-1 + x, 1 + y, -1 + z);
        // face v6-v5-v4
        glVertex3f(-1 + x, 1 + y, -1 + z);
        glVertex3f(1 + x, 1 + y, -1 + z);
        glVertex3f(1 + x, -1 + y, -1 + z);
    }
    glEnd();
    glEndList();
}

void cleanup()
{
    textures.cleanup();
    sfml_textures.cleanup();
    images.cleanup();
    sprites.cleanup();
    fonts.cleanup();
    models.cleanup();
    // shaders.cleanup();
    ui_elem.cleanup();
}

} // anonymous namespace

//---------------------------------------------------------------------------

main_menu::main_menu(game& the_game)
    : game_state(the_game)
    , servers_(get_server_list("http://hexahedra.net/server_list.json"))
    , logo_img_(images("menu_logo") ? *images("menu_logo") : sf::Texture())
    , logo_(logo_img_)
    , copyright_("Copyright (C) 2014, Nocte", font_, 14)
    , time_(0)
    , active_menu_(0)
    , exit_(false)
{
    game_.relative_mouse(false);

    copyright_.setFont(*fonts("default"));
    logo_img_.setSmooth(true);

    menus_.emplace_back(
        std::vector<button>{button{L"Singleplayer"}, button{L"Multiplayer"},
                            button{L"Settings"}, button{L"Exit"}});
    menus_.emplace_back(std::vector<button>());

    auto& buttons(menus_[0]);

    buttons[0].on_clicked = [&] {
        host_ = "";
        port_ = 15556;
        done();
    };
    buttons[1].on_clicked = [&] { switch_menu(1); };
    buttons[2].on_clicked = [&] { std::cout << "Settings" << std::endl; };
    buttons[3].on_clicked = [&] {
        exit_ = true;
        cleanup();
        done();
    };

    for (auto& s : servers_) {
        std::wstring tmp;
        sf::Utf8::toWide(s.name.begin(), s.name.end(), std::back_inserter(tmp),
                         '*');
        menus_[1].push_back(button(tmp));
        menus_[1].back().on_clicked = [&] {
            host_ = s.host;
            port_ = s.port;
            done();
        };
    }
    menus_[1].push_back(button(L"< Back"));
    menus_[1].back().on_clicked = [&] { switch_menu(0); };

    setup_dl();
}

void main_menu::resize(unsigned int x, unsigned int y)
{
    width_ = x;
    height_ = y;

    // float logo_scale (float(x - 100) / (float)logo_img_.getSize().x);
    // logo_.setScale(logo_scale, logo_scale);
    logo_.setPosition(50, 30);
    copyright_.setPosition(0, y - 20);

    int spacing(80);

    for (auto& menu : menus_) {
        int py((y - menu.size() * spacing) * 0.5);
        for (auto& btn : menu) {
            // btn.set_position(x * 0.5, py);
            btn.set_position(170, py);
            py += spacing;
        }
    }
}

void main_menu::update(double time_delta)
{
    time_ += time_delta;
}

void main_menu::switch_menu(int i)
{
    if (i == active_menu_)
        return;

    active_menu_ = i;
    event ev(event::mouse_move_abs, game_.mouse_pos());
    process_event(ev);
}

bool main_menu::process_event(const event& ev)
{
    switch (ev.type) {
    case event::window_close:
        exit_ = true;
        done();
        break;

    default:
        ; // ignore
    }

    for (auto& btn : menus_[active_menu_])
        btn.process_event(ev);

    return true;
}

game_state::transition main_menu::next_state() const
{
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    window().popGLStates();

    if (exit_)
        return game_state::transition();

    auto view_dist(global_settings["viewdist"].as<unsigned int>());

    return game_state::transition(
        std::make_unique<main_game>(game_, host_, port_, view_dist), false);
}

void main_menu::render()
{
    if (exit_)
        return;

    float step(time_ + 40.f);

    glClearColor(0, 0, 0, 255);
    glCheck(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    glCheck(glMatrixMode(GL_PROJECTION));
    glCheck(glLoadIdentity());
    glCheck(gluPerspective(60, (float)width_ / (float)height_, 1.0, 1000.));
    glCheck(glMatrixMode(GL_MODELVIEW));
    glCheck(glLoadIdentity());
    glCheck(gluLookAt(0, 0, -10, 0, 0, 0, std::sin(step * 0.01),
                      std::cos(step * 0.01), 0));
    glCheck(glDepthFunc(GL_LEQUAL));

    glCheck(glEnable(GL_LIGHTING));
    glCheck(glDisable(GL_TEXTURE_2D));
    glCheck(glEnable(GL_DEPTH_TEST));
    glCheck(glDisable(GL_BLEND));
    glCheck(glDisable(GL_CULL_FACE));

    glCheck(glTranslatef(0, 0, -std::fmod(step, 200)));
    glCheck(glCallList(cubes));
    glCheck(glTranslatef(0, 0, 200));
    glCheck(glCallList(cubes));
    glCheck(glTranslatef(0, 0, 200));
    glCheck(glCallList(cubes));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, width_, height_, 0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    auto& win(window());

    sf::RectangleShape rect({340.f, static_cast<float>(height_)});
    rect.setFillColor(sf::Color(10, 10, 10, 80));
    win.draw(rect);
    win.draw(logo_);
    win.draw(copyright_);

    for (auto& btn : menus_[active_menu_])
        btn.draw(win);
}

void main_menu::expose()
{
    game_state::expose();
    game_.relative_mouse(false);

    window().pushGLStates();

    static const GLfloat fogColor[4] = {0, 0, 0, 1.f};
    static const GLfloat ambColor[4] = {0.01f, 0.01f, 0.01f, 1.f};
    static const GLfloat dffColor[4] = {1.f, 1.f, 1.f, 1.f};
    static const GLfloat lightpos[4] = {2.f, 2.f, 2.f, 1.f};

    glCheck(glEnable(GL_NORMALIZE));

    glCheck(glFogi(GL_FOG_MODE, GL_LINEAR));
    glCheck(glFogfv(GL_FOG_COLOR, fogColor));
    glCheck(glFogf(GL_FOG_DENSITY, 1.0f));
    glCheck(glHint(GL_FOG_HINT, GL_NICEST));
    glCheck(glFogf(GL_FOG_START, .5f));
    glCheck(glFogf(GL_FOG_END, 200.0f));
    glCheck(glEnable(GL_FOG));

    glCheck(glEnable(GL_LIGHTING));
    glCheck(glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE));
    glCheck(glEnable(GL_COLOR_MATERIAL));
    glCheck(glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE));
    glCheck(glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE));
    glCheck(glLightfv(GL_LIGHT0, GL_AMBIENT, ambColor));
    glCheck(glLightfv(GL_LIGHT0, GL_DIFFUSE, dffColor));
    glCheck(glLightfv(GL_LIGHT0, GL_POSITION, lightpos));
    glCheck(glEnable(GL_LIGHT0));
}

} // namespace hexa
