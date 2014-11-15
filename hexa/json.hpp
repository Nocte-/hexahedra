//---------------------------------------------------------------------------
/// \file   hexa/json.hpp
/// \brief  Some JSON utility functions
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
#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem/path.hpp>

namespace hexa
{

typedef boost::property_tree::ptree json_data;

json_data read_json(const boost::filesystem::path& file);

void write_json(const json_data& json, const boost::filesystem::path& file);

std::string to_string (const json_data& data);

json_data to_json (const std::string& data);

} // namespace hexa
