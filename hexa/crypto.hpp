//---------------------------------------------------------------------------
/// \file   hexa/crypto.hpp
/// \brief  Convenience wrappers around Crypto++
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

#include <iostream>
#include <vector>

#include <boost/property_tree/ptree.hpp>
#include <cryptopp/aes.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/modes.h>

namespace hexa
{
namespace crypto
{

typedef std::vector<uint8_t> buffer;

//---------------------------------------------------------------------------
// PRNG

buffer make_random(int bytes);
buffer make_aes_key();

std::string hex(const buffer& in);
buffer unhex(const std::string& in);

//---------------------------------------------------------------------------
// Asymmetric key

typedef CryptoPP::DL_PrivateKey_EC<CryptoPP::ECP> private_key;
typedef CryptoPP::DL_PublicKey_EC<CryptoPP::ECP>  public_key;


private_key make_new_key();

public_key get_public_key (const private_key& priv);


std::string
serialize_private_key(const private_key& key);

private_key
deserialize_private_key(const std::string& key);


std::string
serialize_public_key(const public_key& key, bool compress = false);

public_key
deserialize_public_key(const std::string& key);


boost::property_tree::ptree to_json(const public_key& key);

public_key from_json(const boost::property_tree::ptree& json);

buffer to_binary(const public_key& key);

public_key public_key_from_binary(const buffer& bin);


std::string encrypt_ecies(const std::string& plaintext, const public_key& key);

std::string decrypt_ecies(const std::string& ciphertext, const private_key& key);


buffer ecdh(const public_key& pubkey, const private_key& privkey);

//---------------------------------------------------------------------------
// Symmetric key

class aes
{
    //CryptoPP::Rijndael::Encryption enc_;
    CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption enc_;

public:
    aes(const buffer& key);

    void encrypt (const buffer& iv, buffer& in);
    void encrypt (const buffer& iv, std::string& in);

    void decrypt (const buffer& iv, buffer& in);
    void decrypt (const buffer& iv, std::string& in);

};


//---------------------------------------------------------------------------
// Hashing

std::string sha256(const std::string& in);

buffer sha256(const buffer& in);

} // namespace crypto
} // namespace hexa

namespace std
{

inline ostream& operator<<(ostream& str, const ::hexa::crypto::private_key& k)
{
    return str << ::hexa::crypto::serialize_private_key(k);
}

inline ostream& operator<<(ostream& str, const ::hexa::crypto::public_key& k)
{
    return str << ::hexa::crypto::serialize_public_key(k);
}

} // namespace std
