//---------------------------------------------------------------------------
/// \file   hexa/log.hpp
/// \brief  Write timestamped messages to a file
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

#include <iostream>
#include <string>
#include <boost/format.hpp>

namespace hexa {

void set_log_output (std::ostream& str);

void log_msg (const std::string& msg);

template <typename par1>
void log_msg (const std::string& msg, const par1& p1)
{
    try
    {
        log_msg((boost::format(msg) % p1).str());
    }
    catch (std::exception& e)
    {
        log_msg((boost::format("cannot log '%1%', %2%: %3%") % msg % p1 % e.what()).str());
    }
}

template <typename par1, typename par2>
void log_msg (const std::string& msg, const par1& p1, const par2& p2)
{
    try
    {
        log_msg((boost::format(msg) % p1 % p2).str());
    }
    catch (std::exception& e)
    {
        log_msg((boost::format("cannot log '%1%', %2%, %3%: %4%") % msg % p1 % p2 % e.what()).str());
    }
}

template <typename par1, typename par2, typename par3>
void log_msg (const std::string& msg, const par1& p1, const par2& p2,
              const par3& p3)
{
    try
    {
        log_msg((boost::format(msg) % p1 % p2 % p3).str());
    }
    catch (std::exception& e)
    {
        log_msg((boost::format("cannot log '%1%', %2%, %3%, %4%: %5%") % msg % p1 % p2 % p3 % e.what()).str());
    }
}

} // namespace hexa

