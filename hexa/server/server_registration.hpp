//---------------------------------------------------------------------------
/// \file   server/server_registration.hpp
/// \brief  Register the server with the master server list.
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
#pragma once

#include <boost/property_tree/ptree.hpp>
#include <hexa/crypto.hpp>

namespace hexa
{

struct server_registration
{
    std::string url;
    std::string uid;
    std::string api_token;
};

boost::property_tree::ptree server_info();

crypto::private_key get_server_private_key();

void use_private_key_from_password(const std::string& password);

/** Register this server. */
server_registration register_server(const std::string& url
                                    = "auth.hexahedra.net");

void ping_server(const server_registration& info);

void go_offline(const server_registration& info);

} // namespace hexa
