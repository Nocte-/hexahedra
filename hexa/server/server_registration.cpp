//---------------------------------------------------------------------------
// server/server_registration.cpp
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

#include "server_registration.hpp"

#include <sstream>

#include <boost/algorithm/hex.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "../algorithm.hpp"
#include "../json.hpp"
#include "../os.hpp"
#include "../rest.hpp"

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;
namespace po = boost::program_options;
using boost::format;
using namespace boost::algorithm;

namespace hexa
{

extern po::variables_map global_settings;
extern fs::path gamedir;

namespace
{

std::string api_token;

std::string base64_encode(const std::string& s)
{
    const std::string base64_padding[] = {"", "==","="};
    namespace bai = boost::archive::iterators;

    std::stringstream os;
    typedef bai::base64_from_binary<bai::transform_width<const char*, 6, 8> > base64_enc;

    std::copy(base64_enc(s.c_str()), base64_enc(s.c_str() + s.size()),
              std::ostream_iterator<char>(os));

    os << base64_padding[s.size() % 3];

    return os.str();
}

fs::path keys_file()
{
    std::string game_name = global_settings["game"].as<std::string>();
    return app_user_dir() / (game_name + "_keys.json");
}

} // anonymous namespace


pt::ptree read_server_info()
{
    return read_json(keys_file());
}

void write_server_info(const pt::ptree& json)
{
    write_json(json, keys_file());
}

pt::ptree server_info()
{
    auto file = keys_file();
    if (!fs::exists(file)) {
        pt::ptree json;
        json.put("privateKey", crypto::serialize_private_key(crypto::make_new_key()));
        write_server_info(json);
        return json;
    }
    return read_server_info();
}

server_registration register_server(const std::string& url)
{
    auto info = server_info();
    auto priv_key = crypto::deserialize_private_key(info.get<std::string>("privateKey"));
    std::string encrypted_key;
    std::string uid;
    auto pub_key = crypto::get_public_key(priv_key);

    if (info.count("uid")) {
        // We have a UID, so this server has been registered earlier, and we can
        // simply fetch the encrypted API token.
        uid = info.get<std::string>("uid");
        auto res = rest::get((format("https://%1%/api/1/servers/%2%/apikey") % url % uid).str());

        if (res.status_code != 200)
            throw std::runtime_error("Cannot obtain an API token from server");

        encrypted_key = res.json.get<std::string>("encryptedApiKey");
    } else {
        // We don't have a UID yet, so let's register our server.
        auto info_file = gamedir / "info.json";
        if (!fs::exists(info_file))
            throw std::runtime_error("Cannot register server: " + info_file.string() + " not found");

        json_data details;
        auto sub = read_json(info_file);
        details.put_child("servers", sub);
        details.put_child("servers.pubkey", crypto::to_json(pub_key));

        if (fs::exists(gamedir / "icon.png"))
            details.put("servers.icon", base64_encode(file_contents(gamedir / "icon.png")));

        auto res = rest::post({(format("https://%1%/api/1/servers") % url).str(), details});
        if (res.status_code != 201) {
            throw std::runtime_error("Cannot register server");
        }

        uid = res.json.get<std::string>("servers.uid");
        encrypted_key = res.json.get<std::string>("encryptedApiKey");
        info.put("uid", uid);
        write_server_info(info);
    }

    api_token = crypto::decrypt_ecies(unhex(encrypted_key), priv_key);
    std::cout << "Got API token: " << api_token << std::endl;

    return { url, uid, api_token };
}

void ping_server(const server_registration& info)
{
    rest::request req ((format("https://%1%/api/1/servers/%2%/ping") % info.url % info.uid).str());
    req.username = info.uid;
    req.password = info.api_token;
    auto res = rest::put(req);
    if (!res.is_ok())
        throw std::runtime_error((format("could not ping server: %1%, %2%") % res.status_code % res.body).str());
}

void go_offline(const server_registration& info)
{
    rest::request req ((format("https://%1%/api/1/servers/%2%/offline") % info.url % info.uid).str());
    req.username = info.uid;
    req.password = info.api_token;
    rest::put(req);
}

} // namespace hexa
