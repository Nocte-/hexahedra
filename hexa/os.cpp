//---------------------------------------------------------------------------
// lib/os.cpp
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

#include "os.hpp"

#include <cstdlib>
#include <boost/config.hpp>

#if (defined(HEXAHEDRA_LINUX))
#  include <unistd.h>
#  include <sys/types.h>
#  include <pwd.h>

#elif (defined(HEXAHEDRA_WINDOWS))

#elif (defined(HEXAHEDRA_MACOS))
#  include <unistd.h>

#endif

namespace fs = boost::filesystem;

namespace hexa {

fs::path app_user_dir()
{
    static fs::path result;

    if (!result.empty())
        return result;

#if (defined(HEXAHEDRA_LINUX) || defined(HEXAHEDRA_BSD))

    static const char* subdir (".hexahedra");
    auto env (::getenv("HOME"));
    if (env)
    {
        result = fs::path(env) / subdir;
    }
    else
    {
        // No $HOME set, fall back to the home dir from the passwd file.
        struct passwd* pw (::getpwuid(::getuid()));
        result = fs::path(pw->pw_dir) / subdir;
    }

#elif (defined(HEXAHEDRA_WINDOWS))

    // Not using SHGet(Known)FolderPath here because of MinGW.

    auto env (::getenv("LOCALAPPDATA"));
    if (!env)
        throw std::runtime_error("%LOCALAPPDATA% not set");

    result = fs::path(env) / "Hexahedra";

#elif (defined(HEXAHEDRA_MACOS))

    auto env (::getenv("HOME"));
    if (!env)
        throw std::runtime_error("$HOME not set");

    result = fs::path(env) / "Library/Application Support/Hexahedra";

#else

    I have no idea how to fetch the home dir on your platform, sorry.

#endif

    return result;
}

} // namespace hexa

