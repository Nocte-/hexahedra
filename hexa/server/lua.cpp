//---------------------------------------------------------------------------
// lua.cpp
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

#include "lua.hpp"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm.hpp>
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
#include <luabind/operator.hpp>

#include <hexa/aabb.hpp>
#include <hexa/area_data.hpp>
#include <hexa/block_types.hpp>
#include <hexa/ray.hpp>
#include <hexa/trace.hpp>
#include <hexa/vector3.hpp>
#include <hexa/voxel_algorithm.hpp>
#include "player.hpp"
#include "network.hpp"
#include "server_entity_system.hpp"

using namespace boost;
using namespace luabind;
namespace fs = boost::filesystem;

namespace hexa {

lua_State* lua::state_ = nullptr;
std::list<luabind::object>               lua::cb_on_login;
std::list<luabind::object>               lua::cb_console;
std::unordered_map<int, luabind::object> lua::cb_on_action;
std::unordered_map<int, luabind::object> lua::cb_stop_action;
std::unordered_map<int, luabind::object> lua::cb_on_place;
std::unordered_map<int, luabind::object> lua::cb_on_remove;
std::unordered_map<int, luabind::object> lua::material_definitions;
world* lua::world_ = nullptr;
network* lua::net_ = nullptr;

void lua::uglyhack(network* n) { net_ = n; }

static material& get_material(uint16_t i) { return material_prop[i]; }

class lua_component
{
public:
    enum storage_t
    {
        st_wfpos, st_vector, st_string, st_yaw_pitch,
        st_float, st_uint16, st_uint, st_entity,
        st_tag
    };

public:
    lua_component(es::storage::component_id id, std::string name,
                  storage_t type)
        : id_(id), name_(std::move(name)), type_(type)
    { }

    int get_id() { return id_; }
    const std::string& get_name() { return name_; }
    int get_type() { return type_; }

    bool operator== (const std::string& name) const
        { return name_ == name; }

public:
    std::list<luabind::object>  on_change;

private:
    es::storage::component_id   id_;
    std::string                 name_;
    storage_t                   type_;
};

static std::vector<lua_component> registered_components
{
    { 0,  "position",   lua_component::st_wfpos },
    { 1,  "velocity",   lua_component::st_vector }
};

class lua_entity
{
    es::storage&            es_;
    es::storage::iterator   id_;

    template <typename t>
    boost::optional<t> get (es::storage::component_id c) const
    {
        trace("get e %1% c %2%", id_->first, (int)c);
        try
        {
            return es_.get<t>(id_, c);
        }
        catch(...)
        {
            trace("get e %1% c %2% failed", id_->first, (int)c);
        }
        return boost::optional<t>();
    }

    template <typename t>
    void set (es::storage::component_id c, const t& value)
    {
        trace ("set c %1% v %2%", (int)c, value);
        es_.set(id_, c, value);
    }

    template <typename t>
    void set_obj (es::storage::component_id c, const luabind::object& obj)
    {
        es_.set(id_, c, object_cast<t>(obj));
    }

public:
    lua_entity(es::storage& es, es::entity id)
        : es_(es)
        , id_(es_.find(id))
    { }

    boost::optional<wfpos> get_position() const
        { return get<wfpos>(entity_system::c_position); }

    void set_position (const wfpos& p)
        { set(entity_system::c_position, p); }

    boost::optional<vector> get_velocity() const
        { return get<vector>(entity_system::c_velocity); }

    void set_velocity (const vector& v)
        { set(entity_system::c_velocity, v); }

    boost::optional<vector> get_force() const
        { return get<vector>(entity_system::c_force); }

    void set_force (const vector& v)
        { set(entity_system::c_force, v); }

    boost::optional<yaw_pitch> get_orientation() const
        { return get<yaw_pitch>(entity_system::c_orientation); }

    boost::optional<vector> get_impact() const
        { return get<vector>(entity_system::c_impact); }

    boost::optional<uint16_t> get_model() const
        { return get<uint16_t>(entity_system::c_model); }

    void set_model (uint16_t v)
        { set(entity_system::c_model, v); }

    boost::optional<std::string> get_name() const
        { return get<std::string>(entity_system::c_name); }

    void set_name (const std::string& v)
        { set(entity_system::c_name, v); }

    boost::optional<yaw_pitch> get_lookat() const
        { return get<yaw_pitch>(entity_system::c_lookat); }


    boost::optional<luabind::object> get (int component_id) const
        { return false; }

    void set (int component_id, const luabind::object& v)
        {
        try
        {
            if (component_id < 0 || component_id >= (int)registered_components.size())
                return;

            switch (registered_components[component_id].get_type())
            {
            case lua_component::st_wfpos:
                set_obj<wfpos>(component_id, v); break;

            case lua_component::st_vector:
                set_obj<vector>(component_id, v); break;

            case lua_component::st_string:
                set_obj<std::string>(component_id, v); break;

            case lua_component::st_yaw_pitch:
                set_obj<yaw_pitch>(component_id, v); break;
            }
        }
        catch (...)
        {
        }
    }

    bool on_ground() const
        {
            try
            {
                return es_.get<vector>(id_, entity_system::c_impact).z > 0;
            }
            catch(...) { }
            return false;
        }

public:
    int hotbar_size() const
    {
        auto hb (get<hotbar>(entity_system::c_hotbar));
        if (!hb)
            return 0;

        return hb->size();
    }

    void resize_hotbar(size_t newsize)
    {
        auto hb (get<hotbar>(entity_system::c_hotbar));
        if (!hb)
            hb = hotbar(newsize);
        else if (hb->size() == newsize)
            return;

        std::cout << "Resize hotbar to " << newsize << std::endl;
        hb->resize(newsize);
        set(entity_system::c_hotbar, *hb);
    }

    hotbar_slot hotbar_get(int i) const
    {
        auto hb (get<hotbar>(entity_system::c_hotbar));
        if (!hb)
            throw std::runtime_error("no hotbar available");

        return hb->at(i);
    }

    void hotbar_set (int i, const hotbar_slot& s)
    {
        auto hb (get<hotbar>(entity_system::c_hotbar));
        if (!hb)
            throw std::runtime_error("no hotbar available");

        std::cout << "Setting hotbar slot " << i << " to " << s.name << std::endl;
        hb->at(i) = s;
        set(entity_system::c_hotbar, *hb);
    }
};

class player_wrapper
{
    player& p_;

public:
    player_wrapper (player& p) : p_ (p) { }

    void change_speed (double dx, double dy, double dz)
    {
        std::cout << "player change speed by " << dz << std::endl;
    }

    wfpos get_position() const { return {p_.position, p_.position_fraction}; }

    //world_coordinates get_position() const { return p_.position; }

    void set_position(const world_coordinates& p) { p_.position = p; }

    vector3<float> get_fposition() const { return p_.position_fraction; }

    void set_fposition(const vector3<float>& p) { p_.position_fraction = p; }

    world_coordinates get_chunk() const { return p_.chunk_position(); }

    uint32_t get_ip() const { return p_.conn->address.host; }

    std::string get_name() const { return p_.name; }

    ray<float> aiming_at() const
    {
        return ray<float>(vector::origin() + p_.position_fraction + vector(0,0,1.7),
                          p_.head_angle);
    }

    yaw_pitch head_angle() const
    {
        return p_.head_angle;
    }

    int hotbar_size() const { return p_.hotbar.size(); }
    void resize_hotbar(size_t newsize)
    {
        if (newsize == p_.hotbar.size())
            return;

        std::cout << "resize hotbar to " << newsize << std::endl;
        p_.hotbar.resize(newsize);
        send_hotbar();
        std::cout << "Sent" << std::endl;
    }

    hotbar_slot& hotbar_get(int i) const { return p_.hotbar.at(i); }

    void hotbar_set (int i, const hotbar_slot& s)
    {
        std::cout << "Setting hotbar slot " << i << " to " << s.name << std::endl;
        p_.hotbar.at(i) = s;
        send_hotbar();
        std::cout << "Sent" << std::endl;
    }

private:
    void send_hotbar()
    {
        msg::player_configure_hotbar hbmsg;
        hbmsg.slots.assign(p_.hotbar.begin(), p_.hotbar.end());
        if (lua::net_)
        {
            lua::net_->send(p_.entity, serialize_packet(hbmsg), hbmsg.method());
        }
    }
};



lua::lua(server_entity_system& entities, world &w)
    : entities_(entities)
{  
    world_ = &w;
    state_ = lua_open();

    luaopen_base(state_);
    luaopen_string(state_);
    luaopen_table(state_);
    luaopen_math(state_);

    luabind::open(state_);

    module(state_)
    [
        def("define_material", define_material),
        def("define_component", define_component),
        def("material", get_material),
        def("material_definition", material_definition),
        def("material_id",  find_material),
        def("change_block", lua::change_block),
        def("change_block", lua::change_block_s),
        def("get_block", get_block),
        def("on_login", on_login),
        def("on_action", on_action),
        def("on_stop_action", on_stop_action),
        def("on_approach", on_approach),
        def("on_component_change", on_component_change),
        def("on_console", on_console),
        def("server_log", server_log),
        def("raycast", raycast),
        def("are_overlapping", are_overlapping<world_coordinates>),
        def("is_inside", is_inside<world_coordinates>),

        class_<world_coordinates>("pos")
            .def(constructor<uint32_t, uint32_t, uint32_t>())
            .def_readwrite("x", &world_coordinates::x)
            .def_readwrite("y", &world_coordinates::y)
            .def_readwrite("z", &world_coordinates::z)
            .def(self + other<world_vector>())
            .def(self == other<world_coordinates>())
            ,
        class_<world_vector>("veci")
            .def(constructor<int32_t, int32_t, int32_t>())
            .def_readwrite("x", &world_vector::x)
            .def_readwrite("y", &world_vector::y)
            .def_readwrite("z", &world_vector::z)
            .def(self + other<world_vector>())
            .def(self == other<world_vector>())
            ,
        class_<vector>("vecf")
            .def(constructor<float, float, float>())
            .def_readwrite("x", &vector::x)
            .def_readwrite("y", &vector::y)
            .def_readwrite("z", &vector::z)
            .def(self + other<vector>())
            .def(self == other<vector>())
            ,
        class_<wfpos>("wfpos")
            .def_readwrite("pos", &wfpos::pos)
            .def_readwrite("frac", &wfpos::frac)
            .def(self + other<vector>())
            .def(self == other<wfpos>())
            ,
        class_<yaw_pitch>("direction")
            .def(constructor<float, float>())
            .def_readwrite("yaw", &yaw_pitch::x)
            .def_readwrite("pitch", &yaw_pitch::y)
            ,
        class_<aabb<world_coordinates>>("box")
            .def(constructor<const world_coordinates&, const world_coordinates&>())
            .def_readwrite("first", &aabb<world_coordinates>::first)
            .def_readwrite("second", &aabb<world_coordinates>::second)
//            .def_readonly("is_correct", &aabb<world_coordinates>::is_correct)
            .def("make_correct", &aabb<world_coordinates>::make_correct)
            ,
            /*
        class_<player_wrapper>("player")
            .def("change_speed",        &player_wrapper::change_speed)
            .property("position",       &player_wrapper::get_position,
                                        &player_wrapper::set_position)
            .property("position_frac",  &player_wrapper::get_fposition,
                                        &player_wrapper::set_fposition)
            .property("chunk",          &player_wrapper::get_chunk)
            .property("head_angle",     &player_wrapper::head_angle)
            .property("aiming_at",      &player_wrapper::aiming_at)
            .property("ip_address",     &player_wrapper::get_ip)
            .property("name",           &player_wrapper::get_name)
            .property("hotbar_size",    &player_wrapper::hotbar_size,
                                        &player_wrapper::resize_hotbar)
            .def("hotbar_get",          &player_wrapper::hotbar_get)
            .def("hotbar_set",          &player_wrapper::hotbar_set)
            ,
                    */
        class_<lua_component>("component")
            .enum_("core_id")
                [
                value("position",   entity_system::c_position),
                value("velocity",   entity_system::c_velocity),
                value("force",      entity_system::c_force),
                value("orientation",entity_system::c_orientation),
                value("boundingbox",entity_system::c_boundingbox),
                value("impact",     entity_system::c_impact),
                value("model",      entity_system::c_model),
                value("name",       entity_system::c_name),
                value("look_at",    entity_system::c_lookat),
                value("free",       server_entity_system::c_last_server_component)
                ]
            .property("id", &lua_component::get_id)
            .property("name", &lua_component::get_name)
            .property("type", &lua_component::get_type)
            ,
        class_<lua_entity>("entity")
            .property("position", &lua_entity::get_position,
                                  &lua_entity::set_position)
            .property("velocity", &lua_entity::get_velocity,
                                  &lua_entity::set_velocity)
            .property("force",    &lua_entity::get_force,
                                  &lua_entity::set_force)
            .property("orientation", &lua_entity::get_orientation)
            .property("impact", &lua_entity::get_impact)
            .property("model", &lua_entity::get_model, &lua_entity::set_model)
            .property("name", &lua_entity::get_name, &lua_entity::set_name)
            .property("look_at", &lua_entity::get_lookat)
            .property("on_ground", &lua_entity::on_ground)
            .property("hotbar_size", &lua_entity::hotbar_size,
                                     &lua_entity::resize_hotbar)
            .def("hotbar_get", &lua_entity::hotbar_get)
            .def("hotbar_set", &lua_entity::hotbar_set)
            ,
        class_<hotbar_slot>("hotbar_slot")
            .def(constructor<int, std::string>())
            .def_readwrite("type", &hotbar_slot::type)
            .def_readwrite("name", &hotbar_slot::name)
            .def_readwrite("tooltip", &hotbar_slot::tooltip)
            ,
        class_<material>("material")
            .property("transparency", &material::transparency)
            .property("light_emission", &material::light_emission)
            .property("solid", &material::is_solid)
            .property("name", &material::name)
            ,
        class_<area_data>("area_data")
            .def("get", &area_data::get)
            .def("set", &area_data::set)
    ];
}

lua::~lua()
{
    cb_on_login.clear();
    cb_on_action.clear();
    cb_on_place.clear();
    cb_on_remove.clear();
    material_definitions.clear();

    lua_close(state_);
}

bool lua::load (const fs::path& script)
{
    return !luaL_loadfile(state_, script.string().c_str()) &&
           !lua_pcall(state_, 0, 0, 0);
}

std::string lua::get_error() const
{
    std::string result(lua_tostring(state_, -1));
    lua_pop(state_, 1);
    return result;
}

void lua::define_material(uint16_t mat_id, const object& specs)
{
    if (mat_id == 0)
        return;

    material_definitions[mat_id] = specs;
    material& data (register_new_material(mat_id));

    for (luabind::iterator i (specs), end; i != end; ++i)
    {
        std::string key = object_cast<std::string>(i.key());

        if (key == "name")
        {
            std::string temp (object_cast<std::string>(*i));
            data.name = temp;
        }

        else if (key == "texture")
        {
            std::vector<std::string> texlist;
            for (luabind::iterator j (*i); j != end; ++j)
                texlist.push_back(object_cast<std::string>(*j));

            data.textures = find_textures(texlist);
        }

        else if (key == "transparency")
            data.transparency = object_cast<float>(*i)*255.;

        else if (key == "emit_light")
            data.light_emission = object_cast<float>(*i)*255.;

        else if (key == "strength")
            data.light_emission = object_cast<unsigned int>(*i);

        else if (key == "is_solid")
            data.is_solid = object_cast<bool>(*i);

        else if (key == "custom_model")
        {
            for (luabind::iterator j (*i), end; j != end; ++j)
            {
                size_t count (0);
                std::vector<unsigned int> values (6);
                std::vector<std::string>  textures;

                for (luabind::iterator k (*j); k != end; ++k, ++count)
                {
                    if (count < 6)
                        values[count] = object_cast<unsigned int>(*k);
                    else if (count == 6)
                    {
                        textures.clear();
                        textures.push_back(object_cast<std::string>(*k));
                    }
                    else if (count < 12)
                        textures.push_back(object_cast<std::string>(*k));
                }

                if (count < 6)
                    throw std::runtime_error("not enough parameters in custom block definition");

                custom_block::value_type part;
                part.box.first  = chunk_index(values[0] % 16,
                                              values[1] % 16,
                                              values[2] % 16);
                part.box.second = chunk_index(values[3] % 16,
                                              values[4] % 16,
                                              values[5] % 16);
                part.box.make_correct();
                part.textures = find_textures(textures);

                trace("custom model for material #%1%, %2%", mat_id, data.name);
                data.model.emplace_back(std::move(part));
            }
        }

        else if (key == "on_place")
        {
            cb_on_place[mat_id] = *i;
        }

        else if (key == "on_remove")
        {
            cb_on_remove[mat_id] = *i;
        }
    }
}

int lua::define_component(const std::string& name, int type)
{
    int component_id (-1);
    auto found (boost::range::find(registered_components, name));
    if (found == registered_components.end())
    {
        component_id = registered_components.size();
      //  registered_components.emplace_back(component_id, name, type);
        found = std::prev(registered_components.end());
/*
        int reg;
        switch (type)
        {
        case lua_component::st_wfpos:
            reg = entities_.register_component<wfpos>(name); break;
        case lua_component::st_vector:
            reg = entities_.register_component<vector>(name); break;
        case lua_component::st_string:
            reg = entities_.register_component<std::string>(name); break;
        case lua_component::st_uint:
            reg = entities_.register_component<uint32_t>(name); break;
        case lua_component::st_uint16:
            reg = entities_.register_component<uint16_t>(name); break;
        case lua_component::st_float:
            reg = entities_.register_component<float>(name); break;
        }
        */
    }

    return component_id;
}

const object&
lua::material_definition(uint16_t id)
{
    return material_definitions[id];
}

std::array<uint16_t, 6>
lua::find_textures (const std::vector<std::string>& textures)
{
    std::array<uint16_t, 6> result;
    std::vector<uint16_t> tex;
    for (auto& texture : textures)
    {
        auto found (texture_names.find(texture));
        if (found == texture_names.end())
        {
            uint16_t new_index (texture_names.size());
            texture_names[texture] = new_index;
            tex.push_back(new_index);
        }
        else
        {
            tex.push_back(found->second);
        }
    }

    switch (tex.size())
    {
        case 0:
            std::fill(result.begin(), result.end(), 0);
            break;

        case 1:
            std::fill(result.begin(), result.end(), tex[0]);
            break;

        case 2:
            result[0] = tex[0];
            result[1] = tex[0];
            result[2] = tex[0];
            result[3] = tex[0];
            result[4] = tex[1];
            result[5] = tex[1];
            break;

        case 3:
            result[0] = tex[0];
            result[1] = tex[0];
            result[2] = tex[0];
            result[3] = tex[0];
            result[4] = tex[1];
            result[5] = tex[2];
            break;

        case 4:
            result[0] = tex[0];
            result[1] = tex[1];
            result[2] = tex[0];
            result[3] = tex[1];
            result[4] = tex[2];
            result[5] = tex[3];
            break;

        case 6:
            for (int k (0); k < 6; ++k)
                result[k] = tex[k];

            break;
    }
    return result;
}

void lua::on_approach(const world_coordinates& p, unsigned int radius_on,
                      unsigned int radius_off, const object& callback)
{
    try
    {
        player lulz;
        player_wrapper plr (lulz);
        call_function<void>(callback, plr);

    }
    catch (luabind::error&)
    {
        std::cerr << "Lua error: " << lua_tostring(state_, -1) << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown error in on_approach" << std::endl;
    }
}

void lua::on_login(const object& callback)
{
    try
    {
        cb_on_login.push_back(callback);
    }
    catch (luabind::error&)
    {
        std::cerr << "Lua error: " << lua_tostring(state_, -1) << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown error in on_login" << std::endl;
    }
}

void lua::on_component_change(int component_id, const object& callback)
{
    try
    {
        if (component_id < 0 || component_id >= (int)registered_components.size())
            return;

        auto& c (registered_components[component_id]);
        c.on_change.push_back(callback);
    }
    catch (luabind::error&)
    {
        std::cerr << "Lua error: " << lua_tostring(state_, -1) << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown error in on_component_changed" << std::endl;
    }
}

void lua::on_console(const object& callback)
{
    try
    {
        cb_console.push_back(callback);
    }
    catch (luabind::error&)
    {
        std::cerr << "Lua error: " << lua_tostring(state_, -1) << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown error in on_console" << std::endl;
    }
}

void lua::player_logged_in(es::entity plr)
{
    try
    {
        lua_entity temp (entities_, plr);
        for (auto& callback : cb_on_login)
            call_function<void>(callback, temp);
    }
    catch (luabind::error&)
    {
        std::cerr << "Lua error: " << lua_tostring(state_, -1) << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown error in player_logged_in" << std::endl;
    }
}

void lua::on_action(int type, const object& callback)
{
    try
    {
        if (callback.is_valid())
            cb_on_action[type] = callback;
        else
            std::cerr << "No valid callback for action " << type << std::endl;
    }
    catch (luabind::error&)
    {
        std::cerr << "Lua error: " << lua_tostring(state_, -1) << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown error in on_action " << type << std::endl;
    }
}

void lua::on_stop_action(int type, const object& callback)
{
    try
    {
        cb_stop_action[type] = callback;
    }
    catch (luabind::error&)
    {
        std::cerr << "Lua error: " << lua_tostring(state_, -1) << std::endl;
    }
    catch (...)
    {
        std::cerr << "Unknown error in on_stop_action" << std::endl;
    }
}

void lua::start_action(es::entity plr, uint8_t button, uint8_t slot,
                       yaw_pitch look, wfpos pos)
{
    auto found (cb_on_action.find(button));
    if (found == cb_on_action.end())
    {
        trace("Action %1% not defined in Lua", (int)button);
        return;
    }
    if (!found->second.is_valid())
    {
        std::cout << "No valid action bound" << std::endl;
        return;
    }
    if (found->second == luabind::object())
    {
        std::cout << "Nil action bound" << std::endl;
        return;
    }

    trace("Calling action %1%", (int)button);
    lua_entity tmp (entities_, plr);
    call_function<void>(found->second, tmp, (int)slot, look, pos);
}

void lua::stop_action(es::entity plr, uint8_t button)
{
    auto found (cb_stop_action.find(button));
    if (found == cb_stop_action.end())
        return;

    trace("Stopping action %1%", (int)button);
    lua_entity tmp (entities_, plr);
    call_function<void>(found->second, tmp);
}

void lua::console(es::entity plr, const std::string& text)
{
    lua_entity tmp (entities_, plr);
    for (auto& cb : cb_console)
        call_function<void>(cb, tmp, text);
}

void lua::change_block(const world_coordinates& p, uint16_t type)
{
    gameworld().change_block(p, type);
}

void lua::change_block_s(const world_coordinates& p, const std::string& type)
{
    gameworld().change_block(p, type);
}

void lua::place_block(const world_coordinates& p, uint16_t type)
{
    gameworld().change_block(p, type);
    auto cb (cb_on_place.find(type));
    if (cb != cb_on_place.end())
        call_function<void>(cb->second, p, type);
}

void lua::place_block_s(const world_coordinates& p, const std::string& type)
{
    place_block(p, find_material(type));
}

uint16_t lua::get_block(const world_coordinates& p)
{
    return gameworld().get_block(p);
}

luabind::object
lua::raycast(const wfpos& origin, const yaw_pitch& dir, float range)
{
    auto blocks (gameworld().raycast(origin, dir, range));
    luabind::object result (newtable(state_));
    result[1] = std::get<0>(blocks);
    result[2] = std::get<1>(blocks);
    return result;
}

void lua::send_console_message(es::entity plr,
                               const std::string &type,
                               const std::string &name,
                               const std::string &text)
{

}

void lua::server_log(const std::string& msg)
{
    std::cout << "Lua log: " << msg << std::endl;
}

} // namespace hexa

