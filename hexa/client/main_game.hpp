//---------------------------------------------------------------------------
/// \file   client/main_game.hpp
/// \brief  The main game state, where the player is actually playing a game.
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
// Copyright 2012-2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <unordered_set>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <hexa/crypto.hpp>
#include <hexa/entity_system.hpp>
#include <hexa/persistent_storage_i.hpp>
#include <hexa/packet.hpp>
#include <hexa/process.hpp>
#include <hexa/serialize.hpp>

#include "chunk_cache.hpp"
#include "game_state.hpp"
#include "hud.hpp"
#include "udp_client.hpp"
#include "renderer_i.hpp"
#include "player.hpp"
#include "scene.hpp"

namespace hexa
{

class packet;

/** The main game state, where the player is actually playing a game. */
class main_game : public game_state, public udp_client
{
public:
    main_game(game& the_game, const std::string& host, uint16_t port,
              unsigned int view_dist = 24);
    ~main_game();

    void receive(packet p) override;
    void login();
    void request_chunk(const chunk_coordinates& pos);

    player& get_player();

    chunk_cache& map() { return *map_; }

    renderer_i& renderer() { return *renderer_; }

    std::unique_ptr<terrain_mesher_i> make_terrain_mesher(vec3i offset);

public:
    std::string name() const override { return "main game"; }
    void update(double time_delta) override;
    void render() override;
    bool process_event(const event&) override;
    bool process_event(const sf::Event&) override;
    void resize(unsigned int x, unsigned int y) override;

    transition next_state() const override;

public:
    void walk(float direction, float speed);

    void action(uint8_t code);
    void action_stop(uint8_t code);

    void set_view_distance(unsigned int dist);

public:
    boost::signals2::signal<void(double)> on_tick;
    void bg_thread();
    void network_thread();

private:
    void process_event_captured(const event&);
    void process_event_uncaptured(const event&);

    void setup_world(const std::string& host, uint16_t port);
    void setup_renderer();

    void stop();
    double elapsed_time();
    void player_controls();
    void player_motion();
    void console_input(const std::u32string& msg);

    void handshake(deserializer<packet>& p);
    void greeting(deserializer<packet>& p);
    void kick(deserializer<packet>& p);
    void time_sync_response(deserializer<packet>& p);
    void define_resources(deserializer<packet>& p);
    void define_materials(deserializer<packet>& p);

    void entity_update(deserializer<packet>& p);
    void entity_update_physics(deserializer<packet>& p);
    void entity_delete(deserializer<packet>& p);
    void surface_update(deserializer<packet>& p);
    void lightmap_update(deserializer<packet>& p);
    void heightmap_update(deserializer<packet>& p);
    void configure_hotbar(deserializer<packet>& p);
    void global_config(deserializer<packet>& p);
    void print_msg(deserializer<packet>& p);

    void on_disconnect() override;

private:
    boost::asio::io_service io_;

    /** Persistent storage for chunk surfaces and the coarse height map. */
    std::unique_ptr<persistent_storage_i> storage_;
    /** A memory cache for the persistent storage. */
    std::unique_ptr<chunk_cache> map_;
    /** It is determined at run time what kind of renderer will be used,
     ** depending on the OpenGL capabilities of the client. */
    std::unique_ptr<renderer_i> renderer_;

    player player_;
    hud hud_;
    chunk_coordinates old_chunk_pos_;
    world_coordinates old_position_;
    vector old_fraction_;
    scene scene_;
    std::atomic<bool> stop_;

    bool in_action_;
    uint16_t last_action_;

    boost::mutex requests_lock_;
    std::unordered_set<chunk_coordinates> requests_;

    boost::thread asio_;
    boost::thread clock_;
    boost::thread network_;
    std::vector<std::string> textures_;

    double elapsed_;
    bool was_standing_;

    entity_system entities_;
    uint32_t player_entity_;

    bool waiting_for_data_;
    mutable bool loading_screen_;
    bool singleplayer_;
    pid_type server_process_;
    bool show_ui_;

    uint32_t ignore_text_;

    crypto::private_key my_private_key_;
    crypto::buffer      server_nonce_;
    crypto::public_key  server_pubkey_;
    crypto::aes         cipher_;
};

} // namespace hexa
