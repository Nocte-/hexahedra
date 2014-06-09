//---------------------------------------------------------------------------
/// \file   drop_privileges.hpp
/// \brief  Drop the privileges of the current process
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

#include <string>

/** Drop the privileges of the current process.
 *  This function will not do anything if the current process is not
 *  running as root.
 * @throw std::runtime_error If any operation could not be completed
 * @param username      The new UID to run as
 * @param chroot_path   Set a chroot jail to this location */
void drop_privileges(const std::string& username,
                     const std::string& chroot_path);
