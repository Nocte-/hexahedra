//---------------------------------------------------------------------------
// hexa/client/loading_screen.cpp
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
// Copyright 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "loading_screen.hpp"
#include <SFML/Graphics.hpp>
#include "opengl.hpp"
#include "sfml_resource_manager.hpp"

namespace hexa {

loading_screen::loading_screen (game& the_game, const bool &waiting,
                                std::string text, double fadeout_seconds)
    : game_state(the_game)
    , waiting_(waiting)
    , time_(0)
    , fade_timer_(0)
    , text_(std::move(text))
    , fadeout_seconds_(fadeout_seconds)
{  }

void loading_screen::update (double time_delta)
{
    if (!waiting_)
    {
        fade_timer_ += time_delta;
        if (fade_timer_ >= fadeout_seconds_)
            done();
    }

    time_ += time_delta;
}

void loading_screen::render()
{
    uint8_t alpha (255 - std::min<int>(255, (fade_timer_ / fadeout_seconds_) * 255));

    window().pushGLStates();

    glCheck(glEnable(GL_CULL_FACE));
    glCheck(glDisable(GL_DEPTH_TEST));
    glCheck(glMatrixMode(GL_PROJECTION));
    glCheck(glLoadIdentity());
    glCheck(gluOrtho2D(0, width(), height(), 0));
    glCheck(glMatrixMode(GL_MODELVIEW));
    glCheck(glLoadIdentity());
    window().resetGLStates();

    sf::RectangleShape rect (sf::Vector2f(width(), height()));
    rect.setPosition(0, 0);
    rect.setFillColor(sf::Color(0, 0, 0, alpha));
    window().draw(rect);

    sf::Text mesg (text_, *fonts("default"), 20);
    auto size (mesg.getLocalBounds());
    mesg.setPosition({(width() - size.width) * 0.5f, height() * 0.4f});
    mesg.setColor(sf::Color(255, 255, 255, alpha));
    window().draw(mesg);

    window().popGLStates();
}

void loading_screen::process_event (const event&)
{
}

game_state::transition loading_screen::next_state() const
{
	return game_state::transition();
}


} // namespace hexa

