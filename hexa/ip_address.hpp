//---------------------------------------------------------------------------
/// \file   hexa/ip_address.hpp
/// \brief  Storage of IPv4 and IPv6 addresses
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
// Copyright 2013-2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <string>

#ifdef WIN32
# include <windows.h>
# include <in6addr.h>
#else
# include <netinet/in.h>
#endif

namespace hexa {

class ip_address : private std::array<uint16_t, 8>
{
    typedef std::array<uint16_t, 8> base;

public:
    using base::operator[];
    using base::begin;
    using base::end;
    using base::cbegin;
    using base::cend;

public:
    ip_address();
    ip_address(uint32_t ipv4);
    ip_address(std::initializer_list<uint16_t> ipv6);
    ip_address(const std::string& addr);
    ip_address(const in6_addr& addr);

    bool is_ipv4() const;
    bool is_unspecified() const;
    bool is_loopback() const;

    bool operator== (const ip_address& c) const
    {
        for (int i (0); i < 8; ++i)
            if ((*this)[i] != c[i])
                return false;

        return true;
    }

    bool operator!= (const ip_address& c) const
        { return !operator==(c); }

public:
    static ip_address unspecified();
    static ip_address loopback();
};

} // namespace hexa

namespace std
{

string to_string(const hexa::ip_address& addr);

} // namespace std

