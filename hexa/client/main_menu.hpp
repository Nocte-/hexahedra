//---------------------------------------------------------------------------
/// \file   client/main_menu.hpp
/// \brief  The game's main menu screen
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

#include <cstdint>
#include <string>
#include <utility>

#include <SFML/Graphics.hpp>
#include "game_state.hpp"
#include "button.hpp"
#include "server_list.hpp"

namespace hexa {

class main_menu : public game_state
{
public:
    main_menu (game& the_game);

    std::string name() const override { return "main menu"; }
    void update (double time_delta) override;
    void render() override;
    void process_event (const event&) override;
    void resize (unsigned int x, unsigned int y) override;
    void expose() override;

    bool is_transparent() const { return false; }

    transition next_state() const;
    void switch_menu(int i);

private:
    std::vector<server_info> servers_;
    sf::Texture logo_img_;
    sf::Sprite  logo_;
    sf::Font    font_;
    sf::Text    copyright_;
    double      time_;

    std::vector<std::vector<button>> menus_;
    int         active_menu_;

    std::string host_;
    uint16_t    port_;

    int         width_;
    int         height_;
    bool        exit_;
};

} // namespace hexa

