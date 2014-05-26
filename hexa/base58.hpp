//---------------------------------------------------------------------------
/// \file   hexa/base58.hpp
/// \brief  Base58 encoding and decoding
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

#include "basic_types.hpp"

#include <string>
#include <stdexcept>

namespace hexa {

class base58_error : public std::runtime_error
{
public:
    base58_error() : std::runtime_error("not a valid base58 encoding") { }
};

binary_data base58_decode (const std::string& in);

std::string base58_encode (const binary_data& in);

} // namespace hexa
