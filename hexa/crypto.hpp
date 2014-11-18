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

#include <boost/filesystem/path.hpp>
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

buffer x_or(const buffer& a, const buffer& b);
buffer concat(const buffer& a, const buffer& b);

//---------------------------------------------------------------------------
// Asymmetric key

typedef CryptoPP::DL_PrivateKey_EC<CryptoPP::ECP> private_key;
typedef CryptoPP::DL_PublicKey_EC<CryptoPP::ECP> public_key;

private_key make_new_key();

bool is_valid(const private_key& priv);
bool is_valid(const public_key& pub);

public_key get_public_key(const private_key& priv);

std::string serialize_private_key(const private_key& key);

private_key deserialize_private_key(const std::string& key);

void save_pkcs8(const boost::filesystem::path& file, const private_key& key);

private_key load_pkcs8(const boost::filesystem::path& file);

std::string serialize_public_key(const public_key& key, bool compress = false);

public_key deserialize_public_key(const std::string& key);

boost::property_tree::ptree to_json(const public_key& key);

public_key from_json(const boost::property_tree::ptree& json);

buffer to_binary(const public_key& key, bool compressed = true);

buffer to_binary(const private_key& key);

public_key public_key_from_binary(const buffer& bin);

private_key private_key_from_binary(const buffer& bin);

std::string encrypt_ecies(const std::string& plaintext, const public_key& key);

std::string decrypt_ecies(const std::string& ciphertext,
                          const private_key& key);

buffer ecdh(const public_key& pubkey, const private_key& privkey);

//---------------------------------------------------------------------------
// Symmetric key

class aes
{
    bool ready_;
    mutable CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption enc_;

public:
    aes()
        : ready_{false}
    {
    }
    aes(const buffer& key) { set_key(key); }
    aes(aes&& move) = default;

    bool is_ready() const { return ready_; }
    void set_key(const buffer& key);

    void encrypt(const buffer& iv, buffer& in) const;
    void encrypt(const buffer& iv, const uint8_t* ptr, size_t bytes,
                 uint8_t* dest) const;

    void decrypt(const buffer& iv, buffer& in) const;
    void decrypt(const buffer& iv, const uint8_t* ptr, size_t bytes,
                 uint8_t* dest) const;
};

//---------------------------------------------------------------------------
// Hashing

std::string sha256(const std::string& in);

buffer sha256(const buffer& in);

} // namespace crypto

bool operator==(const crypto::public_key& lhs, const crypto::public_key& rhs);

inline bool operator!=(const crypto::public_key& lhs,
                       const crypto::public_key& rhs)
{
    return !(lhs == rhs);
}

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
