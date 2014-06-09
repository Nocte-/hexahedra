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

#include <string>
#include <memory>
#include <SFML/Window.hpp>
#include "game.hpp"

namespace sf
{
class Event;
}

namespace hexa
{

struct event;

/** Base class for game states.
 * Games usually have a few high-level states, such as a title screen, a
 * main menu, the actual game, or a pause screen.  These states are managed
 * by a \a game object. */
class game_state
{
public:
    /** A game state can pass this struct to the game object to transition
     ** to a different state.
     * @sa game_state::transition() */
    struct transition
    {
        /** The default transition is just to return to the previously
         ** active game state. */
        transition()
            : state(nullptr)
            , replace_current(false)
        {
        }

        /** Transition to a new state.
         * @param s  The new game state
         * @param r  Set this to true if the new state has to replace the
         *           current one.  Otherwise, the current state will remain
         *           on the stack and the new state will be pushed on top.
         */
        transition(std::unique_ptr<game_state>&& s, bool r)
            : state(std::move(s))
            , replace_current(r)
        {
        }

#ifdef _MSC_VER
        transition(transition&& m)
            : state(std::move(m.state))
            , replace_current(m.replace_current)
        {
        }
#else
        transition(transition&&) = default;
#endif

        transition(const transition&) = delete;

        std::unique_ptr<game_state> state;
        bool replace_current;
    };

public:
    game_state(game& the_game)
        : game_(the_game)
        , is_done_(false)
    {
    }

    virtual ~game_state() {}

    virtual std::string name() const = 0;

    /** Gets called every frame.
     * @param time_delta  Time difference between this frame and the previous
     *                    one, in seconds */
    virtual void update(double time_delta) = 0;

    /** Implement the render code here. */
    virtual void render() = 0;

    /** Process an event, such as a mouse move or a keypress.
     * @return True if the event was processed, false if the event should
     *         be passed on to the next game state in the stack. */
    virtual bool process_event(const event&) = 0;

    virtual bool process_event(const sf::Event&) { return false; }

    /** Gets called when the window was resized. */
    virtual void resize(unsigned int x, unsigned int y) {}

    /** Another game state finished, and this state gets exposed as
     ** a consequence. */
    virtual void expose() { is_done_ = false; }

    /** Override this to return true if the game state is transparent.
     *  A good example of a transparent state is a 'paused' screen, or
     *  an important notification. */
    virtual bool is_transparent() const { return false; }

    /** Return true if this game state is finished. */
    bool is_done() const { return is_done_; }

    /** After is_done() has returned true, the game object will call this
     ** function to determine how to transition into the next state. */
    virtual transition next_state() const = 0;

public: // Some convenience functions
    sf::RenderWindow& window() const { return game_.window(); }

    bool key_pressed(key code) const { return game_.key_pressed(code); }

    bool mouse_button_pressed(unsigned int button) const
    {
        return game_.mouse_button_pressed(button);
    }

    float joystick_pos(unsigned int axis) const
    {
        return game_.joystick_pos(axis);
    }

    unsigned int width() const { return game_.width(); }
    unsigned int height() const { return game_.height(); }

protected:
    void done() { is_done_ = true; }

protected:
    game& game_;
    bool is_done_;
};

} // namespace hexa
