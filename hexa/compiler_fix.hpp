//---------------------------------------------------------------------------
/// \file   compiler_fix.hpp
/// \brief  Compiler workarounds.
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
// Copyright 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#if (defined(__clang__))

#elif (defined(_MSC_VER))
#  define constexpr const
#  define noexcept 

#elif (defined(__GNUC__))

#include <memory>

namespace std {

/** "That C++11 doesn’t include make_unique is partly an oversight, and it
 **  will almost certainly be added in the future." -- Herb Sutter */
template<typename type, typename... args>
unique_ptr<type> make_unique(args&&... params)
{
    return unique_ptr<type>(new type(forward<args>(params)...));
}

}

#endif

