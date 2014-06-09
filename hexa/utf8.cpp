//---------------------------------------------------------------------------
// hexa/utf8.cpp
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

#include "utf8.hpp"

#include <utf8.h>

namespace hexa
{

std::string utf32_to_utf8(const std::u32string& in)
{
    std::string result;
    utf8::utf32to8(in.begin(), in.end(), std::back_inserter(result));
    return result;
}

std::u32string utf8_to_utf32(const std::string& in)
{
    std::u32string result;
    utf8::utf8to32(in.begin(), in.end(), std::back_inserter(result));
    return result;
}

} // namespace hexa
