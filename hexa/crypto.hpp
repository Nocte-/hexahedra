//---------------------------------------------------------------------------
/// \file   hexa/crypto.hpp
/// \brief  Common cryptographic functions
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

#include <vector>
#include <cryptopp/eccrypto.h>

namespace hexa {
namespace crypto {

CryptoPP::DL_PrivateKey_EC<CryptoPP::ECP>
make_new_key();

std::vector<uint8_t>
make_random (int bytes);

CryptoPP::Integer
make_random_128();

std::string
serialize_private_key (const CryptoPP::DL_PrivateKey_EC<CryptoPP::ECP> & key);

CryptoPP::DL_PrivateKey_EC<CryptoPP::ECP>
deserialize_private_key (const std::string & key);

std::string
serialize_public_key (const CryptoPP::DL_PrivateKey_EC<CryptoPP::ECP> & privkey);

CryptoPP::DL_PublicKey_EC<CryptoPP::ECP>
deserialize_public_key (const std::string & privkey);

}} // namespace hexa::crypto
