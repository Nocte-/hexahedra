//---------------------------------------------------------------------------
// hexa/trace.cpp
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
#ifndef NDEBUG

#include "trace.hpp"

#include <iostream>
#include <mutex>
#include <boost/format.hpp>

#include "log.hpp"

using boost::format;

#ifdef __MINGW32__

// std::to_string is not implemented in MinGW32.  Dafuq?

namespace std {

string to_string(int i)
{
    if (i < 0)
        return string("-") + to_string(-i);

    string result;
    do
    {
        result.insert(result.begin(), "0123456789"[i % 10]);
        i /= 10;
    } while (i != 0);
    return result;
}

}

#endif

namespace hexa {

namespace {
std::mutex debug_trace_mutex;
}

void trace_impl(const char* func, const char* file, unsigned int line,
                const std::string& msg)
{
    std::lock_guard<std::mutex> g (debug_trace_mutex);
    std::cout << file << ':' << line << " (" << func << ") : " << msg << std::endl;
    //log_msg("%1%:%2%  %3%", file, line, msg);
}

void trace_impl_s(const char* func, const char* file, unsigned int line,
                  const std::string& msg, const std::string& a)
{
    try
    {
        std::lock_guard<std::mutex> g (debug_trace_mutex);
        std::cout << file << ':' << line << " (" << func << ") : "
            << (format(msg) % a).str() << std::endl;

        //log_msg("%1%:%2%  %3%", file, line, (format(msg) % a).str());
    }
    catch(...)
    {
        trace_impl(func, file, line, "bad trace message");
    }
}

void trace_impl_s(const char* func, const char* file, unsigned int line,
                  const std::string& msg, const std::string& a,
                  const std::string& b)
{
    try
    {
        std::lock_guard<std::mutex> g (debug_trace_mutex);
        std::cout << file << ':' << line << " (" << func << ") : "
            << (format(msg) % a % b).str() << std::endl;

        //log_msg("%1%:%2%  %3%", file, line, (format(msg) % a % b).str());
    }
    catch(...)
    {
        trace_impl(func, file, line, "bad trace message");
    }
}

void trace_impl_s(const char* func, const char* file, unsigned int line,
                  const std::string& msg, const std::string& a,
                  const std::string& b, const std::string& c)
{
    try
    {
        std::lock_guard<std::mutex> g (debug_trace_mutex);
        std::cout << file << ':' << line << " (" << func << ") : "
            << (format(msg) % a % b %c).str() << std::endl;

        //log_msg("%1%:%2%  %3%", file, line, (format(msg) % a % b % c).str());
    }
    catch(...)
    {
        trace_impl(func, file, line, "bad trace message");
    }
}

} // namespace hexa

#endif
