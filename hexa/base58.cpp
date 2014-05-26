//---------------------------------------------------------------------------
// hexa/base58.hpp
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
// Copyright 2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "base58.hpp"

#include <cassert>
#include <cstring>

// Based on the Bitcoin implementation of Base58, distributed under the
// MIT/X11 license.
// Copyright 2014, The Bitcoin developers
// https://github.com/bitcoin/bitcoin

namespace hexa {

static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

binary_data
base58_decode (const std::string& in)
{
    const char* psz (&in.c_str()[0]);

    // Skip and count leading '1's.
    int zeroes = 0;
    while (*psz == '1') {
        zeroes++;
        psz++;
    }

    // Allocate enough space in big-endian base256 representation.
    binary_data b256 (strlen(psz) * 733 / 1000 + 1); // log(58) / log(256), rounded up.

    // Process the characters.
    while (*psz)
    {
        // Decode base58 character
        const char *ch = strchr(pszBase58, *psz);
        if (ch == NULL)
            throw base58_error();

        // Apply "b256 = b256 * 58 + ch".
        int carry = ch - pszBase58;
        for (std::vector<unsigned char>::reverse_iterator it = b256.rbegin(); it != b256.rend(); it++)
        {
            carry += 58 * (*it);
            *it = carry % 256;
            carry /= 256;
        }
        assert(carry == 0);
        psz++;
    }

    if (*psz != 0)
        throw base58_error();

    // Skip leading zeroes in b256.
    auto it (b256.begin());
    while (it != b256.end() && *it == 0)
        ++it;

    // Copy result into output vector.
    binary_data vch;
    vch.reserve(zeroes + (b256.end() - it));
    vch.assign(zeroes, 0x00);
    while (it != b256.end())
        vch.push_back(*(it++));

    return vch;
}

std::string
base58_encode (const binary_data& in)
{
    const uint8_t* pbegin (&*in.begin());
    const uint8_t* pend   (&*in.end());

    // Skip & count leading zeroes.
    int zeroes = 0;
    while (pbegin != pend && *pbegin == 0)
    {
        pbegin++;
        zeroes++;
    }
    // Allocate enough space in big-endian base58 representation.
    binary_data b58((pend - pbegin) * 138 / 100 + 1); // log(256) / log(58), rounded up.
    // Process the bytes.
    while (pbegin != pend)
    {
        int carry = *pbegin;
        // Apply "b58 = b58 * 256 + ch".
        for (auto it (b58.rbegin()); it != b58.rend(); ++it)
        {
            carry += 256 * (*it);
            *it = carry % 58;
            carry /= 58;
        }
        assert(carry == 0);
        pbegin++;
    }
    // Skip leading zeroes in base58 result.
    auto it (b58.begin());
    while (it != b58.end() && *it == 0)
        it++;

    // Translate the result into a string.
    std::string str;
    str.reserve(zeroes + (b58.end() - it));
    str.assign(zeroes, '1');
    while (it != b58.end())
        str += pszBase58[*(it++)];

    return str;
}

} // namespace hexa
