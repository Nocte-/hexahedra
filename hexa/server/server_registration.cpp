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
#include "../log.hpp"
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

bool have_privkey = false;
crypto::private_key privkey;
std::string api_token;

std::string base64_encode(const std::string& s)
{
    const std::string base64_padding[] = {"", "==", "="};
    namespace bai = boost::archive::iterators;

    std::stringstream os;
    typedef bai::base64_from_binary<bai::transform_width<const char*, 6, 8>>
        base64_enc;

    std::copy(base64_enc(s.c_str()), base64_enc(s.c_str() + s.size()),
              std::ostream_iterator<char>(os));

    os << base64_padding[s.size() % 3];

    return os.str();
}

fs::path info_file()
{
    std::string game_name = global_settings["game"].as<std::string>();
    return app_user_dir() / (game_name + "_serverinfo.json");
}

fs::path key_file()
{
    std::string game_name = global_settings["game"].as<std::string>();
    return app_user_dir() / (game_name + "_privatekey.der");
}

} // anonymous namespace

void use_private_key_from_password(const std::string& password)
{
    if (fs::exists(key_file()))
        throw std::runtime_error(
            (format("A private key already exists in '%1%'.")
             % key_file().string()).str());

    if (password.size() < 16)
        throw std::runtime_error(
            "Passphrase has to be at least 16 characters long.");

    // Keep feeding the passphase through SHA-256 until we get a
    // valid private key.
    crypto::buffer round;
    std::copy(password.begin(), password.end(), std::back_inserter(round));
    for (;;) {
        round = crypto::sha256(round);
        auto key = crypto::private_key_from_binary(round);
        if (crypto::is_valid(key)) {
            have_privkey = true;
            privkey = key;
            break;
        }
    }

    // If we already have a public key stored, check it against
    // the key we just made.
    auto file = info_file();
    if (fs::exists(file)) {
        auto json = read_json(file);
        if (json.count("pubkey")
            && crypto::from_json(json.get_child("pubkey"))
               != crypto::get_public_key(privkey))
            throw std::runtime_error(
                "Passphrase does not match against the public key in "
                + file.string());
    }
}

pt::ptree read_server_info()
{
    return read_json(info_file());
}

void write_server_info(const pt::ptree& json)
{
    write_json(json, info_file());
}

crypto::private_key get_server_private_key()
{
    if (have_privkey)
        return privkey;

    auto file = key_file();
    if (!fs::exists(file)) {
        privkey = crypto::make_new_key();
        crypto::save_pkcs8(file, privkey);
    } else {
        privkey = crypto::load_pkcs8(key_file());
    }
    have_privkey = true;
    return privkey;
}

pt::ptree server_info()
{
    auto file = info_file();
    if (!fs::exists(file)) {
        auto pub = crypto::get_public_key(get_server_private_key());
        pt::ptree json;
        json.put_child("pubkey", crypto::to_json(pub));
        write_server_info(json);
        return json;
    }
    return read_server_info();
}

server_registration register_server(const std::string& url)
{
    using namespace crypto;

    auto info = server_info();
    auto priv_key = get_server_private_key();
    std::string encrypted_key;
    std::string id;
    auto pub_key = get_public_key(priv_key);

    log_msg("Registering server at %1%", url);
    std::string base = (format("https://%1%/api/1/servers") % url).str();

    if (info.count("id")) {
        // We have an ID, so this server has been registered earlier, and we
        // can simply fetch the encrypted API token.
        id = info.get<std::string>("id");

        log_msg("Previously registered as server id %1%, fetching public key "
                "and API token", id);
        auto srv_info = rest::get((format("%1%/%2%?fields=pubkey") % base % id).str());
        if (srv_info.status_code != 200) {
            throw std::runtime_error("Cannot download server info from "
                                     + url);
        }

        if (srv_info.json.count("pubkey") > 0
            && from_json(srv_info.json.get_child("pubkey")) != pub_key) {
            throw std::runtime_error(
                "Your public key doesn't match the one from " + url
                + ". This probably means you've lost your private key.");
        }
        auto res = rest::get((format("%1%/%2%/apikey") % base % id).str());
        if (res.status_code != 200)
            throw std::runtime_error("Cannot obtain an API token from " + url);

        encrypted_key = res.json.get<std::string>("encryptedApiKey");
    } else {
        // We don't have a UID yet, so let's register our server.
        log_msg("Not yet registered, posting data to %1%", url);

        auto info_file = gamedir / "info.json";
        if (!fs::exists(info_file)) {
            throw std::runtime_error("Cannot register server: "
                                     + info_file.string() + " not found");
        }
        json_data details;
        auto sub = read_json(info_file);
        details.put_child("servers", sub);
        details.put_child("servers.pubkey", to_json(pub_key));
        details.put("servers.players.max",
                    global_settings["max-players"].as<unsigned int>());

        std::string hostname{global_settings["hostname"].as<std::string>()};
        if (!hostname.empty()) {
            details.put("servers.connection.host", hostname);
        }
        log_msg("Registering as hostname '%1%'", hostname);
        details.put("servers.connection.port",
                    global_settings["port"].as<unsigned short>());

        if (fs::exists(gamedir / "icon.png")) {
            details.put("servers.icon",
                        base64_encode(file_contents(gamedir / "icon.png")));
        }
        log_msg("POST JSON: %1%", to_string(details));
        auto res = rest::post({base, details});
        if (res.status_code != 201) {
            throw std::runtime_error(
                (format("Cannot register server on %1%: %2%") % url % res.body)
                    .str());
        }
        try {
            id = res.json.get<std::string>("servers.id");
            encrypted_key = res.json.get<std::string>("encryptedApiKey");
        } catch (...) {
            throw std::runtime_error(
                "Server did not return a server ID and api token");
        }
        info.put("id", id);
        write_server_info(info);
        log_msg("Registered new server as ID %1%", id);
    }

    api_token = decrypt_ecies(boost::algorithm::unhex(encrypted_key), priv_key);
    log_msg("Got API token: %1%", api_token);

    return {url, id, api_token};
}

void ping_server(const server_registration& info)
{
    rest::request req((format("https://%1%/api/1/servers/%2%/ping") % info.url
                       % info.uid).str());
    req.username = info.uid;
    req.password = info.api_token;
    auto res = rest::put(req);
    if (!res.is_ok())
        throw std::runtime_error((format("could not ping server: %1%, %2%")
                                  % res.status_code % res.body).str());
}

void go_offline(const server_registration& info)
{
    rest::request req((format("https://%1%/api/1/servers/%2%/ping") % info.url
                       % info.uid).str());
    req.username = info.uid;
    req.password = info.api_token;
    std::cout << "Go offline: " << rest::del(req).status_code << std::endl;
}

} // namespace hexa
