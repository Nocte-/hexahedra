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

#if (defined(_MSC_VER))
#define constexpr const
#define noexcept
#endif

//---------------------------------------------------------------------------

#if (defined(__GNUC__) || defined(__clang__))

#include <memory>

namespace std
{

/** "That C++11 doesn’t include make_unique is partly an oversight, and it
 **  will almost certainly be added in the future." -- Herb Sutter */
template <typename Type, typename... Args>
unique_ptr<Type> make_unique(Args&&... params)
{
    return unique_ptr<Type>(new Type(forward<Args>(params)...));
}
}

#endif
