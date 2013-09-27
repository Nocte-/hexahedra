//---------------------------------------------------------------------------
/// \file   hexa/trace.hpp
/// \brief  A simple trace function for debugging
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

#pragma once

#include <string>

#ifdef __MINGW32__

namespace std {
string to_string (int i);
}

#endif

namespace hexa {

#ifdef NDEBUG
#  define trace(...) ((void)0)
#else
#  define trace(...) (trace_impl(__FUNCTION__, __FILE__, __LINE__, __VA_ARGS__))

void trace_impl(const char* func, const char* file, unsigned int line,
                const std::string& msg);

void trace_impl_s(const char* func, const char* file, unsigned int line,
                  const std::string& msg, const std::string& a);

void trace_impl_s(const char* func, const char* file, unsigned int line,
                  const std::string& msg, const std::string& a,
                  const std::string& b);


template <typename ta> inline
void trace_impl(const char* func, const char* file, unsigned int line,
                  const std::string& msg, const ta& a)
{
    trace_impl_s(func, file, line, msg, std::to_string(a));
}

template <> inline
void trace_impl(const char* func, const char* file, unsigned int line,
                const std::string& msg, const std::string& a)
{
    trace_impl_s(func, file, line, msg, a);
}

template <typename ta, typename tb> inline
void trace_impl(const char* func, const char* file, unsigned int line,
                const std::string& msg, const ta& a, const tb& b)
{
    trace_impl_s(func, file, line, msg, std::to_string(a), std::to_string(b));
}

template <> inline
void trace_impl(const char* func, const char* file, unsigned int line,
                const std::string& msg, const std::string& a, const std::string& b)
{
    trace_impl_s(func, file, line, msg, a, b);
}

template <typename tb> inline
void trace_impl(const char* func, const char* file, unsigned int line,
                const std::string& msg, const std::string& a, const tb& b)
{
    trace_impl_s(func, file, line, msg, a, std::to_string(b));
}

template <typename ta> inline
void trace_impl(const char* func, const char* file, unsigned int line,
                const std::string& msg, const ta& a, const std::string& b)
{
    trace_impl_s(func, file, line, msg, std::to_string(a), b);
}




#endif

} // namespace hexa
