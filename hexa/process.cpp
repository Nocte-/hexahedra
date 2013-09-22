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
#include "log.hpp"

using boost::format;
namespace fs = boost::filesystem;

#if defined(BOOST_POSIX_API)

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

namespace hexa {

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

    if (::kill(id.internal_, SIGTERM) == -1)
        return false;

    int status (0);
    ::waitpid(id.internal_, &status, 0);
    return true;
}

void
kill_process (pid_type id)
{
    if (id.internal_ != 0)
    {
        ::kill(id.internal_, SIGKILL);
        int status (0);
        ::waitpid(id.internal_, &status, 0);
    }
}

}

#elif defined(BOOST_WINDOWS_API)

namespace hexa {

pid_type
start_process (const boost::filesystem::path &exe,
               const std::vector<std::string>& args)
{
    PROCESS_INFORMATION proc_info;
    ZeroMemory(&proc_info, sizeof(proc_info));

    STARTUPINFO start_info;
    ZeroMemory(&start_info, sizeof(start_info));
    start_info.cb = sizeof(start_info);

    std::string cmdline_params;
    for (auto& arg : args)
    {
        if (!cmdline_params.empty())
            cmdline_params.push_back(' ');

        cmdline_params += arg;
    }
    // Can't use c_str() here, add our own null terminator.
    cmdline_params.push_back(0);

    auto launch (exe);
    if (launch.extension() != ".exe")
        launch += ".exe";

    log_msg(launch.string());
    if (!CreateProcess(launch.string().c_str(), &cmdline_params[0],
                       nullptr, nullptr,
                       FALSE, CREATE_BREAKAWAY_FROM_JOB, nullptr, nullptr,
                       &start_info, &proc_info))
    {
        throw std::runtime_error((format("CreateProcess failed, error code %1%") % GetLastError()).str());
    }

    // Set up a Job, so the child process is ended after the parent
    // exits.
    /*
    HANDLE job_obj (CreateJobObject(0, 0));
    if (job_obj)
    {
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION ext_info;
        ZeroMemory(&ext_info, sizeof(ext_info));
        ext_info.BasicLimitInformation.LimitFlags
                = 0x00002000; // JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE

        if (SetInformationJobObject(job_obj, JobObjectExtendedLimitInformation,
                                    &ext_info, sizeof(ext_info)) == 0)
        {
            throw std::runtime_error((format("SetInformationJobObject failed, error code %1%") % GetLastError()).str());
        }
        if (AssignProcessToJobObject(job_obj, proc_info.hProcess) == 0)
        {
            throw std::runtime_error((format("AssignProcessToJobObject failed, error code %1%") % GetLastError()).str());
        }
    }
    else
    {
        log_msg("Cannot create JobObject, child process might not exit.");
    }
    */

    return proc_info;
}

bool
terminate_process (pid_type id)
{
    GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, id.dwProcessId);
    return CloseHandle(id.hThread) && CloseHandle(id.hProcess);
}

}

#endif
