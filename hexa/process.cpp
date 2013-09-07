//---------------------------------------------------------------------------
// hexa/process.cpp
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

#include "process.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/filesystem/operations.hpp>

using boost::format;
namespace fs = boost::filesystem;

#if defined(BOOST_POSIX_API)

#include <sys/types.h>
#include <signal.h>

void
zombie_handler (int code)
{
    //std::cout << "Child exited with code " << code << std::endl;
}

pid_type
start_process (const boost::filesystem::path &exe,
               const std::vector<std::string>& args)
{
    if (!fs::exists(exe))
        throw std::runtime_error((format("program file '%1%' does not exist") % exe.string()).str());

    auto pid (::fork());
    if (pid == -1)
    {
        throw std::runtime_error((format("fork() failed for '%1%'") % exe.string()).str());
    }
    else if (pid == 0)
    {
        std::vector<char*> argv;
        for (auto& arg : args)
        {
            auto buf (new char[arg.size() + 1]);
            std::copy(arg.begin(), arg.end(), buf);
            buf[arg.size()] = 0;
            argv.push_back(buf);
        }

        argv.push_back(nullptr);
        ::execv(exe.string().c_str(), &argv[0]);

        for (auto ptr : argv)
            delete [] ptr;

        throw std::runtime_error("execve() failed");
    }
    else
    {
        ::signal(SIGCHLD, zombie_handler);
    }
    return pid;
}

bool
terminate_process (pid_type id)
{
    if (id.internal_ == 0)
        return false;

    return ::kill(id.internal_, SIGTERM) != -1;
}

void
kill_process (pid_type id)
{
    if (id.internal_ != 0)
        ::kill(id.internal_, SIGKILL);
}

#elif defined(BOOST_WINDOWS_API)

pid_type
start_process (const boost::filesystem::path &exe,
               const std::vector<std::string>& args)
{
    PROCESS_INFORMATION proc_info;
    ZeroMemory(&proc_info, sizeof(proc_info));

    STARTUPINFO start_info;
    start_info.cb = sizeof(start_info);

    std::string cmdline_params;
    for (auto& arg : args)
    {
        if (!cmdline_params.empty())
            cmdline_params.push_back(' ');

        cmdline_params += arg;
    }
    cmdline_params.push_back(0);

    if (!CreateProcess(exe.string().c_str(), &cmdline_params[0],
                       nullptr, nullptr,
                       FALSE, CREATE_NO_WINDOW, nullptr, nullptr,
                       &start_info, &proc_info))
    {
        throw std::runtime_error((format("CreateProcess failed, error code %1%") % GetLastError()).str());
    }

    return proc_info;
}

bool
terminate_process (pid_type id)
{
    return CloseHandle(id.hThread) && CloseHandle(id.hProcess);
}

#endif



