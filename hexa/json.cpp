//---------------------------------------------------------------------------
// hexa/json.cpp
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
// Copyright (C) 2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#include "json.hpp"

#include <fstream>
#include <sstream>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>

using namespace boost::algorithm;
namespace pt = boost::property_tree;

namespace hexa
{

json_data read_json(const boost::filesystem::path& file)
{
    std::ifstream str(file.string());
    if (!str)
        throw std::runtime_error(std::string("cannot open '") + file.string()
                                 + "' for reading");

    pt::ptree json;
    pt::read_json(str, json);
    return json;
}

void write_json(const json_data& json, const boost::filesystem::path& file)
{
    std::ofstream str(file.string());
    if (!str)
        throw std::runtime_error(std::string("cannot open '") + file.string()
                                 + "' for writing");

    pt::write_json(str, json);
}

std::string to_string(const json_data& data)
{
    std::stringstream str;
    pt::write_json(str, data);
    std::string result{str.str()};
    erase_all(result, "\r");
    erase_all(result, "\n");
    erase_all(result, "\t");
    return result;
}

json_data to_json(const std::string& data)
{
    json_data result;
    std::istringstream str{data};
    pt::read_json(str, result);
    return result;
}

} // namespace hexa
