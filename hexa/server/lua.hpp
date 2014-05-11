//---------------------------------------------------------------------------
/// \file   server/lua.hpp
/// \brief  Interface with Lua scripts.
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

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <cstdarg>
#include <list>
#include <tuple>
#include <unordered_map>
#include <luabind/function.hpp>
#include <luabind/object.hpp>
#include <boost/filesystem/path.hpp>

#include <hexa/basic_types.hpp>
#include <hexa/ip_address.hpp>
#include <hexa/ray.hpp>
#include "network.hpp"

namespace hexa {

class player;
class server_entity_system;
class world;
class network;

class lua
{
public:
    lua (server_entity_system& entities, world& w);
    ~lua();

    bool load (const boost::filesystem::path& script);

    std::string get_error() const;

    static void
    define_material(uint16_t material, const luabind::object& data);

    static int
    define_component(const std::string& name, int type);

    static const luabind::object&
    material_definition(uint16_t material);

    static void
    on_authenticate_player(const luabind::object& callback);

    static void
    on_approach(const world_coordinates& p, unsigned int radius,
                unsigned int radius_off, const luabind::object& callback);

    static void
    on_action(int type, const luabind::object& callback);

    static void
    on_stop_action(int type, const luabind::object& callback);

    static void
    on_login(const luabind::object& callback);

    static void
    on_component_change(int component_id, const luabind::object& callback);

    static void
    on_console(const luabind::object& callback);


    static void
    change_block(const world_coordinates& p, uint16_t type);

    static void
    change_block_s(const world_coordinates& p, const std::string& type);

    static void
    place_block(const world_coordinates& p, uint16_t type);

    static void
    place_block_s(const world_coordinates& p, const std::string& type);

    static uint16_t
    get_block(const world_coordinates& p);

    static luabind::object
    raycast(const wfpos& origin, const yaw_pitch& dir, float range);

    static int material_id(const std::string& name);

    static void
    send_console_message(es::entity plr,
                         const std::string& json);

    static void
    broadcast_console_message(const std::string& json);

    static void
    server_log(const std::string& msg);

    template <class ret>
    ret call (const std::string& func, ...)
    {
        va_list va;
        return luabind::call_function<ret>(state_, func.c_str(), va);
    }

    template <class arg1, class arg2, class arg3>
    void call_void(const std::string& func, arg1 a1, arg2 a2, arg3& a3)
    {
        luabind::call_function<void>(state_, func.c_str(), a1, a2, a3);
    }

    static lua_State* state() { return state_; }

public:
    void player_logged_in (es::entity plr);

    void start_action(es::entity plr, uint8_t button, uint8_t slot,
                      yaw_pitch look, wfpos pos);

    void stop_action(es::entity plr, uint8_t button);

    void console(es::entity plr, const std::string& text);

    bool authenticate(const ip_address& source, const std::string& player,
                      const std::string& passwd);

    static std::array<uint16_t, 6>
         find_textures (const std::vector<std::string>& textures);

    void uglyhack (network*);
    static network* net_;

private:
    static world& gameworld() { return *world_; }

private:
    static lua_State* state_;
    static std::unordered_map<int, luabind::object> cb_on_action;
    static std::unordered_map<int, luabind::object> cb_stop_action;
    static std::unordered_map<int, luabind::object> cb_on_place;
    static std::unordered_map<int, luabind::object> cb_on_remove;
    static std::unordered_map<int, luabind::object> material_definitions;
    static std::list<luabind::object>               cb_on_login;
    static std::list<luabind::object>               cb_console;
    static luabind::object                          cb_authenticate_player;

    server_entity_system&   entities_;

    // hack
    static world*           world_;
};

} // namespace hexa

