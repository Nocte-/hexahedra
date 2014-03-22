//---------------------------------------------------------------------------
/// \file   client/loading_screen.hpp
/// \brief  Shown while the client is connecting to the server
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

#include "game_state.hpp"

namespace hexa {

/** A simple game state that just shows "connecting...", and fades out
 ** to the main game state when it is done. */
class loading_screen : public game_state
{
public:
    loading_screen (game& the_game, const bool& waiting,
                    std::string text = "Connecting ...",
                    double fadeout_seconds = 0.5);

    std::string name() const { return "loading screen"; }

    void update (double time_delta);
    void render();
    bool process_event (const event&);
    bool is_transparent() const { return true; }
    transition next_state() const;

    void start_fading();

private:
    const bool& waiting_;
    double      time_;
    double      fade_timer_;
    std::string text_;
    double      fadeout_seconds_;
};

} // namespace hexa

