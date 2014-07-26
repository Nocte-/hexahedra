//---------------------------------------------------------------------------
/// \file   server/random.hpp
/// \brief  Hash and PRNG functions for terrain generation.
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
#pragma once

namespace hexa
{

/** Fowler-Noll-Vo 1a hash function.
 * @param in    Pointer to start of byte array
 * @param count Number of bytes to hash
 * @return FNV-1a  hash value of the input */
inline const uint32_t fnv_hash(const uint8_t* in, uint32_t count)
{
    uint32_t hash(2166136261);
    for (; count > 0; --count)
        hash = (hash ^ *in++) * 16777619;

    return hash;
}

template <typename t>
inline const uint32_t fnv_hash(const t& in)
{
    return fnv_hash(reinterpret_cast<const uint8_t*>(&in), sizeof(t));
}

inline const uint32_t fnv_hash(const std::string& in)
{
    return fnv_hash(reinterpret_cast<const uint8_t*>(&in[0]), sizeof(in));
}

/** Pseudorandom number generator.
 *  This is the standard glibc linear congruential generator.
 * @param i  Seed value, or previous round of the PRNG
 * @return  Next random value */
inline const uint32_t prng(const uint32_t i)
{
    return (1103515245 * i + 12345) & 0x7FFFFFFF;
}

/** Use the PRNG to generate a random 32-bit integer coordinate.
 * @param n  PRNG value
 * @return A random position based on the next 3 numbers from the PRNG */
inline const chunk_coordinates prng_next_pos(uint32_t& n)
{
    auto x(prng(n));
    auto y(prng(x));
    auto z(prng(y));
    n = z;

    return {x, y, z};
}

/** Get the next number from the PRNG. */
inline const uint32_t prng_next(uint32_t& n)
{
    return n = prng(n);
}

/** Get a floating point number ranged -1..1 from the PRNG. */
inline const float prng_next_f(uint32_t& n)
{
    n = prng(n);
    return static_cast<int32_t>(n << 1) / 2147483647.f;
}

/** Get a double ranged 0..1 from the PRNG. */
inline const double prng_next_zto(uint32_t& n)
{
    n = prng(n);
    return n / 2147483647.0;
}

} // namespace hexa
