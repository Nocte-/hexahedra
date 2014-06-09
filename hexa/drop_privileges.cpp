//---------------------------------------------------------------------------
/// \file   drop_privileges.cpp
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
// Copyright 2011, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "drop_privileges.hpp"

#include <iostream>
#include <errno.h>
#include <string.h>
#include <stdexcept>

#ifndef _WIN32
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#endif

void drop_privileges(const std::string& username,
                     const std::string& chroot_path)
{
///\todo Find a suitable Win32 alternative

#ifndef _WIN32
    if (getuid() != 0 && geteuid() != 0)
        return; // Not root anyway, no need to drop privileges.

    if (access(chroot_path.c_str(), F_OK) || access(chroot_path.c_str(), X_OK))
        throw std::runtime_error("cannot access chroot jail");

    struct passwd* pw_ent(getpwnam(username.c_str()));
    if (!pw_ent)
        throw std::runtime_error(std::string("cannot find user ") + username);

    if (chroot(chroot_path.c_str()) != 0)
        throw std::runtime_error("cannot chroot()");

    if (chdir("/") != 0)
        throw std::runtime_error("cannot switch working directory");

    if (setuid(pw_ent->pw_uid) < 0)
        throw std::runtime_error(std::string("cannot switch to user ")
                                 + username);
#endif
}
