//---------------------------------------------------------------------------
// lib/win32_minidump.cpp
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

#include "win32_minidump.hpp"

// Checking for _MSC_VER instead of Win32, since MinGW doesn't support this.
#ifdef _MSC_VER

#include <boost/format.hpp>
#include <boost/filesystem/operations.hpp>

#include <Windows.h>
#include <dbghelp.h>
#include <shellapi.h>
#include <shlobj.h>

#include "os.hpp"

#pragma comment(lib, "dbghelp.lib")

using namespace boost;

namespace {
    std::string appname_;
}

LONG WINAPI GenerateDump(EXCEPTION_POINTERS* pointers)
{
    SYSTEMTIME local_time;
    MINIDUMP_EXCEPTION_INFORMATION param;
    boost::filesystem::create_directory(hexa::app_user_dir());
    GetLocalTime(&local_time);
    
    std::string filename (
        (format("%s\\%s-%04d%02d%02d-%02d%02d%0d2.dmp")
          % hexa::app_user_dir().string()
          % appname_
          % local_time.wYear % local_time.wMonth % local_time.wDay
          % local_time.wHour % local_time.wMinute % local_time.wSecond).str());

    param.ThreadId = GetCurrentThreadId();
    param.ExceptionPointers = pointers;
    param.ClientPointers = TRUE;

	HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 
							  0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 

    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                      hFile,
                      MiniDumpWithProcessThreadData, &param,
                      nullptr, nullptr);

    std::string msg((format("Something went terribly, terribly wrong.  "
        "Information about this crash has been saved to a dump file: \n%s\n\n"
        "You can help fixing this bug by sending this file to "
        "hexahedra.maintainer@gmail.com .  Thanks!") % filename).str());
 
    MessageBox(0, msg.c_str(), "Hexahedra", MB_ICONWARNING);

    return EXCEPTION_EXECUTE_HANDLER;
}

void setup_minidump(const std::string& appname)
{
    appname_ = appname;
    SetUnhandledExceptionFilter(GenerateDump);
}

#else // ifdef MSC_VER

void setup_minidump(const std::string&) {} 

#endif

