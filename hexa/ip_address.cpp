//---------------------------------------------------------------------------
// hexa/ip_address.cpp
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

#include "ip_address.hpp"

#include <stdexcept>

namespace hexa {

static const std::array<uint16_t, 6> ipv4_mask {{ 0, 0, 0, 0, 0, 0xffff }};

ip_address::ip_address() { std::fill(begin(), end(), 0); }

ip_address::ip_address(std::initializer_list<uint16_t> ipv6)
{
    if (ipv6.size() != 8)
        throw std::invalid_argument("ipv6 addresses must be 16 bytes long");

    std::copy(ipv6.begin(), ipv6.end(), begin());
}

ip_address::ip_address(uint32_t ipv4)
{
    std::copy(ipv4_mask.begin(), ipv4_mask.end(), begin());
    (*this)[6] = static_cast<uint16_t>(ipv4 >> 16);
    (*this)[7] = static_cast<uint16_t>(ipv4      );
}

ip_address::ip_address(const std::string& addr)
{

}

ip_address ip_address::unspecified()
{
    return ip_address({ 0, 0, 0, 0, 0, 0, 0, 0 });
}

ip_address ip_address::loopback()
{
    return ip_address({ 0, 0, 0, 0, 0, 0, 0, 1 });
}

bool ip_address::is_ipv4() const
{
    return std::equal(ipv4_mask.begin(), ipv4_mask.end(), begin());
}

bool ip_address::is_unspecified() const
{
    return *this == ip_address::unspecified();
}

bool ip_address::is_loopback() const
{
    return *this == ip_address::loopback();
}

} // namespace hexa

//---------------------------------------------------------------------------

namespace std
{

string to_string (const hexa::ip_address& addr)
{
    string result;

    return result;
}

}

