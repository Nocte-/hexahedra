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

#ifdef _MSC_VER

#include <boost/format.hpp>
#include <Windows.h>
#include <dbghelp.h>
#include <shellapi.h>
#include <shlobj.h>

using namespace boost;

#pragma comment(lib, "dbghelp.lib")

LONG WINAPI GenerateDump(EXCEPTION_POINTERS* pointers)
{
    SYSTEMTIME local_time;
    MINIDUMP_EXCEPTION_INFORMATION param;

    GetLocalTime(&local_time);
    
    std::string filename (
        (format("hexaclient-%04d%02d%02d-%02d%02d%0d2.dmp")
          % local_time.wYear % local_time.wMonth % local_time.wDay
          % local_time.wHour % local_time.wMinute % local_time.wSecond).str());

    param.ThreadId = GetCurrentThreadId();
    param.ExceptionPointers = pointers;
    param.ClientPointers = TRUE;

	HANDLE hFile = CreateFile( filename.c_str(), GENERIC_READ | GENERIC_WRITE, 
							  0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 

    BOOL success (MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                                    hFile,
                                    MiniDumpWithProcessThreadData, &param,
                                    nullptr, nullptr));

    MessageBox(0, "Something went terribly, terribly wrong.  The folder that "
                  "contains hexaclient.exe now also contains a *.dmp file.  "
                  "You can help fixing this bug by sending this file to " 
                  "hexahedra.maintainer@gmail.com .  Thanks!", 
                  "Hexahedra client", MB_ICONWARNING);

    return EXCEPTION_EXECUTE_HANDLER;
}

void setup_minidump()
{
    SetUnhandledExceptionFilter(GenerateDump);
}

#else

void setup_minidump() {} 

#endif

