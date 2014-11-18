//---------------------------------------------------------------------------
// hexa/client/player_info.cpp
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
// Copyright 2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "player_info.hpp"

#include <boost/filesystem.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <hexa/base58.hpp>
#include <hexa/crypto.hpp>
#include <hexa/os.hpp>

namespace fs = boost::filesystem;
namespace po = boost::program_options;
namespace pt = boost::property_tree;

namespace hexa
{

extern po::variables_map global_settings;

namespace
{

fs::path filename_()
{
    return app_user_dir() / "player_info.json";
}

fs::path keyfile()
{
    return app_user_dir() / "player_private_key.der";
}

void generate_new_key(player_info& info)
{
    auto key(crypto::make_new_key());

    info.public_key = crypto::get_public_key(key);
    info.private_key = key;
}

void generate_new_uid(player_info& info)
{
    info.uid = base58_encode(crypto::make_random(8));
}

} // anonymous namespace

//---------------------------------------------------------------------------


player_info::player_info()
{
}

player_info::player_info (const boost::property_tree::ptree& json)
    : uid(json.get<std::string>("uid"))
    //, public_key{crypto::from_json(json.get_child("pubkey"))}
    //, private_key{crypto::deserialize_private_key(json.get<std::string>("private_key"))}
    , private_key{crypto::load_pkcs8(keyfile())}
{
    public_key = crypto::get_public_key(private_key);
}

player_info get_player_info()
{
    if (!fs::exists(filename_())) {
        player_info new_info;
        generate_new_key(new_info);
        generate_new_uid(new_info);
        write_player_info(new_info);
        return new_info;
    }

    pt::ptree json;
    pt::json_parser::read_json(filename_().string(), json);
    return { json };
}

void write_player_info(const player_info& info)
{
    pt::ptree json;
    json.put("uid", info.uid);
    //json.put_child("pubkey", crypto::to_json(info.public_key));
    //json.put("private_key", crypto::serialize_private_key(info.private_key));

    pt::json_parser::write_json(filename_().string(), json);
    fs::permissions(filename_(), fs::perms::owner_read);

    crypto::save_pkcs8(keyfile(), info.private_key);
    fs::permissions(keyfile(), fs::perms::owner_read);
}

} // namespace hexa
