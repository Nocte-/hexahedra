//---------------------------------------------------------------------------
// client/server_list.cpp
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
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "server_list.hpp"

#include <sstream>
#include <boost/format.hpp>
#include <boost/filesystem/operations.hpp>

#include "../base58.hpp"
#include "../config.hpp"
#include "../json.hpp"
#include "../log.hpp"
#include "../rest.hpp"

using boost::format;
using namespace boost::property_tree;
namespace fs = boost::filesystem;

namespace hexa
{

server_list_online::server_list_online(const std::string& url)
    : root_{"https://" + url + "/api/1"}
{
}

ptree server_list_online::get_list(int start, int count)
{
    ptree result;
    std::string qry;
    if (start >= 0 && count > 0)
        qry = (format("?start=%1%&count=%2%") % start % count).str();

    auto res = rest::get(root_ + "/servers" + qry);
    if (res.status_code == 200)
        return std::move(res.json);

    return result;
}

std::string get_url_or_empty(const std::string& url)
{
    auto res = rest::get(url);
    if (res.status_code == 200)
        return std::move(res.body);

    return {};
}

std::string server_list_online::get_icon(const std::string& server_id)
{
    return get_url_or_empty(root_ + "/servers/" + server_id + "/icon");
}

std::string server_list_online::get_screenshot(const std::string& screenshot_id)
{
    return get_url_or_empty(root_ + "/screenshots/" + screenshot_id);
}



game_list_local::game_list_local(const fs::path& rootdir)
    : root_{rootdir}
{
    if (!fs::is_directory(root_))
        throw std::runtime_error(rootdir.string() + " is not a directory");
}

ptree screenshot_list(const fs::path& dir)
{
    ptree list;
    for (int j = 1; j < 6; ++j) {
        std::string img{"screenshot" + std::to_string(j) + ".jpeg"};
        fs::path scr_file{dir / img};
        if (fs::is_regular_file(scr_file)) {
            ptree item;
            item.put("", dir.filename().string() + "/" + img);
            list.push_back(std::make_pair("", item));
        }
    }
    return list;
}

ptree game_list_local::get_list(int start, int count)
{
    ptree list;
    typedef fs::directory_iterator diriter;
    for (auto i = diriter(root_); i != diriter(); ++i) {
        fs::path info_file{*i / "info.json"};
        if (!fs::is_regular_file(info_file))
            continue;

        try {
            ptree game = read_json(info_file);
            game.put("id", i->path().filename().string());
            game.put_child("screenshots", screenshot_list(*i));
            list.push_back(std::make_pair("", game));
        } catch (std::exception& e) {
            log_msg("Failed to parse '%1%': %2%", info_file.string(), e.what());
        }
    }

    ptree result;
    result.add_child("servers", std::move(list));
    return result;
}

std::string game_list_local::get_icon(const std::string& server_id)
{
    try {
        return file_contents(root_ / server_id /  "icon.png");
    } catch(...) {
    }
    return {};
}

std::string game_list_local::get_screenshot(const std::string& screenshot_id)
{
    try {
        return file_contents(root_ / screenshot_id);
    } catch(...) {
    }
    return {};
}


} // namespace hexa
