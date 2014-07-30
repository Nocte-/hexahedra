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
#include <curl/curl.h>
#include <boost/property_tree/ptree.hpp>
#include <hexa/config.hpp>

using namespace boost::property_tree;

namespace hexa
{

bool check_version(const std::string& version)
{
    if (version.empty())
        return true;

    uint32_t maj(0), min(0), pat(0);

    if (sscanf(version.c_str(), "%u.%u.%u", &maj, &min, &pat) != 3) {
        if (sscanf(version.c_str(), "%u.%u", &maj, &min) != 2)
            return false;

        pat = 0;
    }

    return PROJECT_VERSION_MAJOR > maj
           || (PROJECT_VERSION_MAJOR == maj
               && (PROJECT_VERSION_MINOR > min
                   || (PROJECT_VERSION_MINOR == min
                       && PROJECT_VERSION_PATCH >= pat)));
}

std::vector<server_info> get_server_list(const std::string& json_uri)
{
    std::vector<server_info> result;
   
    
    return result;
/*
    http::client::request rq(json_uri);
    rq << header("Connection", "close");
    http::client temp_client;

    ptree tree;
    std::string body{http::body(temp_client.get(rq))};
    std::stringstream str(body);
    read_json(str, tree);
    for (auto& n : tree.get_child("servers")) {
        if (!check_version(n.second.get<std::string>("client_version", "")))
            continue;

        server_info item;
        item.host = n.second.get<std::string>("host");
        item.port = n.second.get<uint16_t>("port");
        item.name = n.second.get<std::string>("name");
        item.desc = n.second.get<std::string>("description");
        item.icon = n.second.get<std::string>("icon");
        item.screenshot = n.second.get<std::string>("screenshot");

        result.push_back(item);
    }

    return result;
    */
}

} // namespace hexa
