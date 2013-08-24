//---------------------------------------------------------------------------
/// \file   hexa/process.hpp
/// \brief  Starting and stopping processes.
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
// Copyright 2012-2013, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once
#include <string>
#include <vector>
#include <boost/system/config.hpp>
#include <boost/filesystem/path.hpp>

#if defined(BOOST_POSIX_API)

#include <unistd.h>

typedef pid_t  pid_type;

#elif defined(BOOST_WINDOWS_API)

#include <windows.h>

typedef PROCESS_INFORMATION  pid_type;

#else

# error "Only Windows and POSIX are supported."

#endif

/** Start a new process in the background.
 * \throw std::runtime_error
 * \param exe   Path to executable
 * \param args  Arguments
 * \return Process handle */
pid_type start_process (const boost::filesystem::path& exe,
                        const std::vector<std::string>& args);

/** Terminate a process.
 * \return true if the process could be terminated succesfully */
bool terminate_process (pid_type id);

/** Forcefully end a process. */
void kill_process (pid_type id);

