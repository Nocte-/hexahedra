//---------------------------------------------------------------------------
/// \file   os.hpp
/// \brief  Operating System abstractions
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
#pragma once

// Defines taken from Boost.  Unfortunately, the way Boost sets up the
// preprocessor isn't useful to me, so I had to duplicate it.

#if (defined(linux) || defined(__linux) || defined(__linux__)                 \
     || defined(__GNU__) || defined(__GLIBC__)) && !defined(_CRAYC)
#define HEXAHEDRA_LINUX

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)     \
    || defined(__DragonFly__)
#define HEXAHEDRA_BSD

#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define HEXAHEDRA_WINDOWS

#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#define HEXAHEDRA_MACOS

#endif

#include <boost/filesystem/path.hpp>

namespace hexa
{

/** Return the user directory for this application.
 *  This is where we'll store configuration files and cache game assets.
 *  Linux: ~/.hexahedra/
 *  Windows: %LOCALAPPDATA%\\Hexahedra
 *  OSX: ~/Libraries/Application Support/Hexahedra */
boost::filesystem::path app_user_dir();

/** Return the full path of the executable.
 *  Linux: /proc/self/exe
 *  Windows: GetModuleFileName()
 */
boost::filesystem::path executable_path();

/** Return the temp directory.
 * Linux: $TEMP (usually /tmp)
 * Windows:
 */
boost::filesystem::path temp_dir();

} // namespace hexa
