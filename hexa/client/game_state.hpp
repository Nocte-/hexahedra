//---------------------------------------------------------------------------
/// \file   client/game_state.hpp
/// \brief  Abstract base class for game states
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

#pragma once

#include <memory>
#include <SFML/Window.hpp>
#include "game.hpp"

namespace sf {
class Event;
}

namespace hexa {

class event;

/** Base class for game states.
 * Games usually have a few high-level states, such as a title screen, a
 * main menu, the actual game, or a pause screen.  These states are managed
 * by a \a game object. */
class game_state
{
public:
    struct transition
    {
        std::unique_ptr<game_state>     state;
        bool                            replace_current;
    };

public:
    game_state (game& the_game) : game_(the_game), is_done_(false) { }

    virtual ~game_state() {}

    virtual void update(double time_delta) = 0;
    virtual void render() = 0;
    virtual void process_event (const event&) = 0;
    virtual void process_event (const sf::Event&) { }

    virtual void resize (unsigned int x, unsigned int y) { }
    virtual void expose() { is_done_ = false; }

    virtual bool is_transparent() const { return false; }
    bool         is_done() const { return is_done_; }

    virtual transition next_state() const = 0;

public: // Some convenience functions

    sf::RenderWindow& window() const
        { return game_.window(); }

    bool key_pressed (key code) const
        { return game_.key_pressed(code); }

    bool mouse_button_pressed (unsigned int button) const
        { return game_.mouse_button_pressed(button); }

    float joystick_pos (unsigned int axis) const
        { return game_.joystick_pos(axis); }

    unsigned int width() const { return game_.width(); }
    unsigned int height() const { return game_.height(); }

protected:
    void done() { is_done_ = true; }

protected:
    game& game_;
    bool  is_done_;
};

} // namespace hexa
