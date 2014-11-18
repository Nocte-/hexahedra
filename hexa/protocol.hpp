//---------------------------------------------------------------------------
/// \file   hexa/protocol.hpp
/// \brief  Definition of the network protocol.
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

#include <string>

#include "basic_types.hpp"
#include "block_types.hpp"
#include "compression.hpp"
#include "hotbar_slot.hpp"
#include "packet.hpp"
#include "serialize.hpp"

namespace hexa
{
namespace msg
{

typedef enum {
    /** Will arrive in the same order as sent. */
    sequenced,
    /** Sent reliably, but the order is undetermined. */
    reliable,
    /** No guarantees. */
    unreliable
} reliability;

/** Interface for all network messages.
 * If you want to know the binary format of a message, just look at the
 * serialize() function.  Variables are sent in network byte order, and
 * the length of variable length data (strings, lists, maps, etc) is
 * encoded as a 16-bit field, followed by the actual data.  Strings are
 * in UTF-8. */
class msg_i
{
public:
    virtual uint8_t type() const = 0;
    virtual reliability method() const { return sequenced; }
};

//---------------------------------------------------------------------------
// Server --> client

/**@defgroup servertoclient Server to client
 * The messages that are sent from the server to the clients. */
/**@{*/

/** Sent every now and then to keep the connection open, and clocks
 ** synchronized. */
class keep_alive : public msg_i
{
public:
    enum { msg_id = 0 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return unreliable; }

    /** Age of the game, measured in 100ths of a second. */
    gameclock_t ticks;

    /** (De)serialize this message. */
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(ticks);
    }
};

/** Greet a client that has just connected. */
class handshake : public msg_i
{
public:
    enum { msg_id = 1 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    handshake()
        : proto_maj{0}
        , proto_min{0}
        , client_maj{0}
        , client_min{0}
    {
    }

    /** Protocol version. */
    uint8_t proto_maj, proto_min;
    /** Minimum client version */
    uint8_t client_maj, client_min;
    /** The server's name. */
    std::string server_name;
    /** The server's ID, or an empty buffer if not registered. */
    binary_data server_id;
    /** Public key. */
    binary_data public_key;
    /** Nonce for deriving a session key */
    binary_data nonce;
    /** More info as JSON. */
    std::string json;

    /** (De)serialize this message. */
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(proto_maj)(proto_min)(client_maj)(client_min)(server_name)(
            server_id)(public_key)(nonce)(json);
    }
};

/** Greet the client after it has identified itself. */
class greeting : public msg_i
{
public:
    enum { msg_id = 2 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    /** The player's starting position. */
    world_coordinates position;
    /** The player's entity ID. */
    uint32_t entity_id;
    /** Game clock. */
    gameclock_t client_time;
    /** Message of the day. */
    std::string motd;

    /** (De)serialize this message. */
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(position)(entity_id)(client_time)(motd);
    }
};

/** Disconnect an unruly client. */
class kick : public msg_i
{
public:
    enum { msg_id = 3 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    /** Message explaining why the player was kicked off the server. */
    std::string reason;

    /** (De)serialize this message. */
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(reason);
    }
};

/** Respond to a time sync message. */
class time_sync_response : public msg_i
{
public:
    enum { msg_id = 4 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return unreliable; }

    gameclock_t request;
    gameclock_t response;

    /** (De)serialize this message. */
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(request)(response);
    }
};

/** The list of used resource names. */
class define_resources : public msg_i
{
public:
    enum { msg_id = 5 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    std::vector<std::string> textures;
    std::vector<std::string> models;
    std::vector<std::string> sounds;

    /** (De)serialize this message. */
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(textures)(models)(sounds);
    }
};

/** Send the client information about the materials we're using. */
class define_materials : public msg_i
{
public:
    struct record
    {
        record() {}
        record(uint16_t m, material d)
            : material_id{m}
            , definition{d}
        {
        }

        uint16_t material_id;
        material definition;

        template <typename Archive>
        Archive& serialize(Archive& ar)
        {
            return ar(material_id)(definition);
        }
    };

public:
    enum { msg_id = 6 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    std::vector<record> materials;

    /** (De)serialize this message. */
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(materials);
    }
};

/** Define a material type with a custom 3-D model. */
class define_custom_blocks : public msg_i
{
public:
    struct part
    {
        chunk_index corner1;
        chunk_index corner2;
        std::array<uint16_t, 6> textures;

        template <typename Archive>
        Archive& serialize(Archive& ar)
        {
            ar(corner1)(corner2);
            for (int i = 0; i < 6; ++i)
                ar(textures[i]);

            return ar;
        }
    };

    struct record
    {
        std::string name;
        std::vector<part> parts;

        template <typename Archive>
        Archive& serialize(Archive& ar)
        {
            return ar(name)(parts);
        }
    };

public:
    enum { msg_id = 7 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    std::vector<record> models;

    /** (De)serialize this message. */
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(models);
    }
};

/** Send updates in the entity system. */
class entity_update : public msg_i
{
public:
    enum { msg_id = 10 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    struct value
    {
        uint32_t entity_id;
        uint16_t component_id;
        binary_data data;

        value() {}

        value(uint32_t e, uint16_t c, binary_data&& d)
            : entity_id{e}
            , component_id{c}
            , data{std::move(d)}
        {
        }

        template <typename Archive>
        Archive& serialize(Archive& ar)
        {
            return ar(entity_id)(component_id)(data);
        }
    };

    std::vector<value> updates;

    /** (De)serialize this message. */
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(updates);
    }
};

/** Special entity update message for physics.
 * This works much like a normal entity update message, but it is
 * unreliable, and only sends the position and velocity.  This makes
 * the packet more compact and faster to parse.  It also has a time
 * stamp, so the client can compensate for lag. */
class entity_update_physics : public msg_i
{
public:
    enum { msg_id = 11 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return unreliable; }

    struct value
    {
        uint32_t entity_id;
        wfpos pos;
        vector velocity;

        value() {}

        value(uint32_t e, wfpos p, vector v)
            : entity_id{e}
            , pos{p}
            , velocity{v}
        {
        }

        template <typename Archive>
        Archive& serialize(Archive& ar)
        {
            return ar(entity_id)(pos)(velocity);
        }
    };

    gameclock_t timestamp;
    std::vector<value> updates;

    /** (De)serialize this message. */
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(timestamp)(updates);
    }
};

/** Remove an entity completely. */
class entity_delete : public msg_i
{
public:
    enum { msg_id = 12 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    uint32_t entity_id;

    /** (De)serialize this message. */
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(entity_id);
    }
};

/** A part of the world's height map. */
class heightmap_update : public msg_i
{
public:
    enum { msg_id = 13 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    struct record
    {
        map_coordinates pos;
        chunk_height height;

        record() {}

        record(map_coordinates p, chunk_height h)
            : pos{p}
            , height{h}
        {
        }

        template <typename Archive>
        Archive& serialize(Archive& ar)
        {
            return ar(pos)(height);
        }
    };

    std::vector<record> data; /**< Array with height data. */

    /** (De)serialize this message. */
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(data);
    }
};

/** The light map of a chunk. */
class lightmap_update : public msg_i
{
public:
    enum { msg_id = 15 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    chunk_coordinates position; /**< Position of the chunk. */
    compressed_data data;       /**< Compressed light data. */

    /** (De)serialize this message. */
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(position)(data);
    }
};

/** The surface and light map of a chunk. */
class surface_update : public msg_i
{
public:
    enum { msg_id = 16 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    chunk_coordinates position; /**< Position of the chunk. */

    compressed_data terrain; /**< Compressed opaque & transparent surfaces. */
    compressed_data light;   /**< Compressed light maps. */

    /** (De)serialize this message. */
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(position)(terrain)(light);
    }
};

/** Register player stat info. */
class player_stat_register : public msg_i
{
public:
    enum { msg_id = 32 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    uint8_t id;
    std::string name;
    std::string desc;

    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(id)(name)(desc);
    }
};

/** Register player stat info. */
class player_stat_update : public msg_i
{
public:
    enum { msg_id = 33 };
    uint8_t type() const { return msg_id; }

    uint8_t id;
    uint32_t value;
    uint32_t max;

    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(id)(value)(max);
    }
};

/** Change the layout of the player's hotbar. */
class player_configure_hotbar : public msg_i
{
public:
    enum { msg_id = 34 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    struct slot : public hotbar_slot
    {
        slot() {}

        slot(hotbar_slot init)
            : hotbar_slot{init}
        {
        }

        slot(hotbar_slot::slot_type t, std::string name)
            : hotbar_slot(t, name)
        {
        }

        template <typename Archive>
        Archive& serialize(Archive& ar)
        {
            return ar(type)(name)(tooltip)(badge)(counter)(progress_bar)(
                can_drag)(cooldown);
        }
    };

    std::vector<slot> slots;

    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(slots);
    }
};

/** Global configuration parameters. */
class global_config : public msg_i
{
public:
    enum { msg_id = 35 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    std::string name;
    std::string value;

    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(name)(value);
    }
};

/** Print a message in the console. */
class print_msg : public msg_i
{
public:
    enum { msg_id = 36 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    std::string json;

    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(json);
    }
};

/**@}*/

//---------------------------------------------------------------------------
// Client --> server

/**@defgroup ClientToServer Client to server
 * The messages that are sent from the client to the server. */
/**@{*/

/** Login. */
class login : public msg_i
{
public:
    enum { msg_id = 128 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    /** Mode (0 = singleplayer localhost, 1 = multiplayer) */
    uint8_t mode;
    /** Player name. */
    std::string name;
    /** Player UID, or empty if not registered. */
    binary_data uid;
    /** Public key. */
    binary_data public_key;
    /** MAC hash (= sha256(s||n)) */
    binary_data mac;
    /** Extra JSON data. */
    std::string json;

    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(mode)(name)(uid)(public_key)(mac)(json);
    }
};

/** Logout. */
class logout : public msg_i
{
public:
    enum { msg_id = 129 };
    uint8_t type() const { return msg_id; }
};

/** This message is sent a few times at the beginning to synchronize the
 ** internal clock with the server's. */
class time_sync_request : public msg_i
{
public:
    enum { msg_id = 130 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return unreliable; }

    gameclock_t request;

    /** (De)serialize this message. */
    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(request);
    }
};

/** The player typed something in the console. */
class console : public msg_i
{
public:
    enum { msg_id = 131 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return sequenced; }

    std::string text;

    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(text);
    }
};

/** Request chunk surface data. */
class request_surfaces : public msg_i
{
public:
    enum { msg_id = 139 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    struct record
    {
        chunk_coordinates position;
        uint32_t version;

        record() {}

        record(chunk_coordinates p, uint32_t v = 0)
            : position(p)
            , version(v)
        {
        }

        template <typename Archive>
        Archive& serialize(Archive& ar)
        {
            return ar(position)(version);
        }
    };

    std::vector<record> requests;

    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(requests);
    }
};

/** Request coarse height map data. */
class request_heights : public msg_i
{
public:
    enum { msg_id = 140 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    struct record
    {
        map_coordinates position;
        gameclock_t last_update;

        record() {}

        record(map_coordinates pos, gameclock_t upd = 0)
            : position(pos)
            , last_update(upd)
        {
        }

        template <typename Archive>
        Archive& serialize(Archive& ar)
        {
            return ar(position)(last_update);
        }
    };

    std::vector<record> requests;

    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(requests);
    }
};

/** Player has started an action (e.g. digging) */
class button_press : public msg_i
{
public:
    enum { msg_id = 160 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return sequenced; }

    button_press() {}
    button_press(uint8_t button_, uint8_t slot_, yaw_pitch look_, wfpos pos_)
        : button{button_}
        , slot{slot_}
        , look{look_}
        , pos{pos_}
    {
    }

    uint8_t button;
    uint8_t slot;
    yaw_pitch look;
    wfpos pos;

    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(button)(slot)(look)(pos);
    }
};

/** Player has stopped performing an action. */
class button_release : public msg_i
{
public:
    enum { msg_id = 161 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return sequenced; }

    button_release(uint8_t button_ = 0) {}

    uint8_t button;

    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(button);
    }
};

/** Player has triggered an action (e.g. casting a spell) */
class trigger : public msg_i
{
public:
    enum { msg_id = 162 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return sequenced; }

    trigger(yaw_pitch look_, uint8_t slot_)
        : look{look_}
        , slot{slot_}
    {
    }

    yaw_pitch look;
    uint8_t slot;

    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(look)(slot);
    }
};

/** Player is looking in a certain direction. */
class look_at : public msg_i
{
public:
    enum { msg_id = 163 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return unreliable; }

    look_at() {}
    look_at(yaw_pitch look_)
        : look(look_)
    {
    }

    yaw_pitch look;

    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(look);
    }
};

/** Player movement. */
class motion : public msg_i
{
public:
    enum { msg_id = 164 };
    uint8_t type() const { return msg_id; }
    reliability method() const { return reliable; }

    /** Move direction (0 = straight ahead, 63 = to the right)
     *  This is relative to the direction the player is facing,
     *  so it's important to keep track of msg::look_at as well. */
    uint8_t move_dir;
    /** Movement speed (0 = standstill, 255 = fastest) */
    uint8_t move_speed;
    /** Current position. */
    wfpos position;

    template <typename Archive>
    void serialize(Archive& ar)
    {
        ar(move_dir)(move_speed)(position);
    }
};

/**@}*/

template <class message_t>
binary_data serialize_packet(message_t& m)
{
    binary_data result;
    result.push_back(message_t::msg_id);
    auto Archive(make_serializer(result));
    m.serialize(Archive);
    return result;
}

} // namespace msg
} // namespace hexa
