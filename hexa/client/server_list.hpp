//---------------------------------------------------------------------------
/// \file   client/server_list.hpp
/// \brief  Retrieve and parse the list of game servers
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

#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem/path.hpp>

namespace hexa
{

/** Interface for accessing game list data. */
class server_list_i
{
public:
    virtual boost::property_tree::ptree get_list(int start = -1, int count = -1) = 0;

    virtual std::string get_icon(const std::string& server_id) = 0;

    virtual std::string get_screenshot(const std::string& screenshot_id) = 0;
};

/** Grab game list data from an online source. */
class server_list_online : public server_list_i
{
public:
    server_list_online(const std::string& url);

    boost::property_tree::ptree get_list(int start = -1, int count = -1) override;
    std::string get_icon(const std::string& server_id) override;
    std::string get_screenshot(const std::string& screenshot_id) override;

private:
    std::string root_;
};

/** Grab game list data from the filesystem. */
class game_list_local : public server_list_i
{
public:
    game_list_local(const boost::filesystem::path& rootdir);

    boost::property_tree::ptree get_list(int start = -1, int count = -1) override;
    std::string get_icon(const std::string& server_id) override;
    std::string get_screenshot(const std::string& screenshot_id) override;

private:
    boost::filesystem::path root_;
};

} // namespace hexa
