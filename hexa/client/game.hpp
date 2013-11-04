//---------------------------------------------------------------------------
/// \file   client/game_state.hpp
/// \brief  Manages game states
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

#include <array>
#include <memory>
#include <vector>
#include <SFML/Graphics.hpp>
#include "../algorithm.hpp"
#include "../compiler_fix.hpp"
#include "../vector2.hpp"
#include "keycodes.hpp"

namespace hexa {

class game_state;

/** Manages game states
 * A title screen, a main menu, and the actual game are examples of
 * game states.  The individual states are implemented in their own
 * classes, \a game only ties them together. */
class game
{
public:
    /** Initialize the game.
     * \param title     Window title
     * \param width     Initial window width
     * \param height    Initial window height */
    game (const std::string& title, unsigned int width, unsigned int height);

    /** Start the event loop, showing this game state. */
    void run (std::unique_ptr<game_state> initial_state);

    /** Push a new state on top of the stack. */
    void push (std::unique_ptr<game_state> new_state);

    /** Create a new game state.
     * \param parameters    List of parameters that will be forwarded to
     *                      the constructor of \a t */
    template <typename t, typename... args>
    std::unique_ptr<game_state> make_state(args&&... parameters)
    {
        return std::make_unique<t>(*this, std::forward<args>(parameters)...);
    }

    sf::RenderWindow& window() { return window_; }

    bool            key_pressed (key code) const;
    bool            mouse_button_pressed (unsigned int button) const;
    float           joystick_pos (unsigned int axis) const;
    vector2<int>    mouse_pos() const;

    unsigned int    width() const { return width_; }
    unsigned int    height() const { return height_; }

    /** Turn relative mouse mode on or off. */
    void            relative_mouse (bool on);

    /** Check if the mouse is currently in relative mode. */
    bool            mouse_is_relative() const { return rel_mouse_; }

    double          total_time_passed() const { return time_; }

private:
    void            poll_events();
    void            handle_keypress(uint32_t keycode);
    void            resize(unsigned int width, unsigned int height);
    void            toggle_fullscreen();

private:
    /** Stack of game states.
     *  Every time a state is finished, it is popped off the stack and
     *  the previous one is exposed. */
    std::vector<std::unique_ptr<game_state>> states_;

    sf::RenderWindow    window_;
    std::string         window_title_;
    unsigned int        width_, height_;

    /** Keeps track of which keys are pressed. */
    std::array<bool, static_cast<unsigned int>(key::count)>    key_pressed_;

    /** Keeps track of the position of joystick axes. */
    std::array<float, 32> joy_axis_;

    /** Joystick button status. */
    std::array<bool, 32> joy_btn_pressed_;

    /** Mouse button status. */
    std::array<bool, 8> mouse_btn_pressed_;

    /** Using relative mouse mode? */
    bool                rel_mouse_;

    /** In fullscreen mode? */
    bool                fullscreen_;

    /** Remember the old window size when switching to fullscreen. */
    unsigned int        window_width_, window_height_;

    /** Current mouse position. */
    vector2<int>        mouse_pos_;

    /** Total time passed. */
    double              time_;
};

} // namespace hexa
