//---------------------------------------------------------------------------
/// \file   hexa/crypto.cpp
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

#include "crypto.hpp"

#include <sstream>
#include <stdexcept>
#include <iostream>

#include <cryptopp/eccrypto.h>
#include <cryptopp/asn.h>
#include <cryptopp/base64.h>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>
#include <cryptopp/oids.h>
#include <cryptopp/osrng.h>

#include <boost/algorithm/hex.hpp>

using namespace CryptoPP;
using namespace boost::algorithm;

namespace hexa
{
namespace crypto
{

static AutoSeededRandomPool rng;

std::string hex(const buffer& in)
{
    static const char* v = "0123456789ABCDEF";
    std::string result;
    for (auto c : in) {
        result.push_back(v[(c >> 4) & 0x0f]);
        result.push_back(v[c & 0x0f]);
    }
    return result;
}

buffer unhex(const std::string &in)
{
    if (in.size() % 2 != 0)
        throw std::runtime_error("unhex: Not a valid hex representation");

    buffer result(in.size() / 2);
    boost::algorithm::unhex(in.begin(), in.end(), result.begin());
    return result;
}

buffer make_random(int bytes)
{
    if (bytes < 1)
        throw std::runtime_error("'bytes' must be greater than zero");

    buffer out(bytes);
    rng.GenerateBlock((byte*)&out[0], bytes);
    return out;
}

buffer make_aes_key()
{
    return make_random(16);
}

Integer make_random_128()
{
    byte out[16];
    rng.GenerateBlock(out, 16);
    return Integer(out, 16);
}



private_key make_new_key()
{
    ECIES<ECP>::Decryptor decr{rng, ASN1::secp256k1()};
    return decr.GetKey();
}

public_key get_public_key (const private_key& priv)
{
    DL_PublicKey_EC<ECP> key;
    priv.MakePublicKey(key);
    return key;
}

std::string serialize_number(const Integer& num, size_t len = 32)
{
    std::string result;
    HexEncoder enc{new StringSink{result}};
    num.Encode(enc, len);
    return result;
}

Integer deserialize_number(const std::string& num)
{
    return Integer(num.c_str());
}

std::string serialize_private_key(const private_key &key)
{
    return serialize_number(key.GetPrivateExponent());
}

private_key deserialize_private_key(const std::string& privkey)
{
    Integer x;
    HexDecoder dec;
    dec.Put(reinterpret_cast<const byte*>(&*privkey.begin()), privkey.size());
    dec.MessageEnd();
    x.Decode(dec, 32);
    private_key result;
    result.Initialize(ASN1::secp256k1(), x);
    return result;
}

std::string serialize_public_key(const public_key& key, bool compress)
{
    auto pt = key.GetPublicElement();
    std::string result;
    HexEncoder enc{new StringSink(result)};
    key.GetGroupParameters().GetCurve().EncodePoint(enc, pt, compress);
    return result;
}

public_key deserialize_public_key(const std::string& key)
{
    throw 0;
}

buffer to_binary(const public_key& key)
{
    buffer result;
    result.resize(33);
    const auto& q = key.GetPublicElement();
    key.GetGroupParameters().GetCurve().EncodePoint((byte*)&result[0], q, true);
    return result;
}

public_key public_key_from_binary(const buffer& bin)
{
    if (bin.size() != 33)
        throw std::runtime_error("Compressed public key must be 33 bytes long");

    if (bin[0] != 2 && bin[0] != 3)
        throw std::runtime_error("Not a valid compressed public key");

    public_key key;
    key.AccessGroupParameters().SetPointCompression(true);
    key.AccessGroupParameters().Initialize(ASN1::secp256k1());
    auto& curve = key.GetGroupParameters().GetCurve();
    ECP::Point p;
    curve.DecodePoint(p, (byte*)&bin[0], 33);
    key.SetPublicElement(p);
    return key;
}


boost::property_tree::ptree to_json(const public_key& key)
{
    boost::property_tree::ptree result;
    const auto& q = key.GetPublicElement();
    result.put("x", serialize_number(q.x));
    result.put("y", serialize_number(q.y));
    return result;
}

public_key from_json(const boost::property_tree::ptree& json)
{
    ECP::Point q;
    q.x = deserialize_number(json.get<std::string>("x"));
    q.y = deserialize_number(json.get<std::string>("y"));

    public_key result;
    result.Initialize(ASN1::secp256k1(), q);
    return result;
}

std::string encrypt_ecies(const std::string& plaintext, const public_key& key)
{
    ECIES<ECP>::Encryptor encr {key};
    std::string cipher;
    try {
        StringSource(plaintext, true, new PK_EncryptorFilter(rng, encr, new StringSink(cipher)));
    } catch (Exception& e) {
        throw std::runtime_error(e.GetWhat());
    }
    return cipher;
}

std::string decrypt_ecies(const std::string& ciphertext, const private_key& key)
{
    if (ciphertext.empty())
        return {};

    ECIES<ECP>::Decryptor decr;
    decr.AccessKey().AccessGroupParameters().Initialize(ASN1::secp256k1());
    decr.AccessKey().SetPrivateExponent(key.GetPrivateExponent());

    std::string plaintext;
    try {
        StringSource(ciphertext, true, new PK_DecryptorFilter(rng, decr, new StringSink(plaintext)));
    } catch (Exception& e) {
        throw std::runtime_error(e.GetWhat());
    }

    return plaintext;
}

void der_encode_public_key(SecByteBlock& enc, const public_key& key)
{
    ByteQueue bq;
    key.DEREncodePublicKey(bq);
    size_t len = bq.MaxRetrievable();
    enc.New(len);
    bq.Get(enc,len);
}

void der_encode_private_key(SecByteBlock& enc, const private_key& key)
{
    auto& exp = key.GetPrivateExponent();
    size_t len = exp.ByteCount();
    enc.CleanNew(len);
    exp.Encode(enc, len);
}

buffer ecdh(const public_key& pubkey, const private_key& privkey)
{
    ECDH<ECP>::Domain dh{ASN1::secp256k1()};
    SecByteBlock priv_exp, pub_int;
    der_encode_public_key(pub_int, pubkey);
    der_encode_private_key(priv_exp, privkey);

    buffer result(dh.AgreedValueLength());
    if (!dh.Agree(&result[0], priv_exp, pub_int)) {
        throw std::runtime_error("ECDH failed");
    }
    return result;
}

aes::aes(const buffer &key)
{
    // Even though we don't use the IV right away, the library
    // will complain if we don't set one straight away.
    static const buffer dummy_iv {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    enc_.SetKeyWithIV(&key[0], key.size(), &dummy_iv[0], dummy_iv.size());
}

void aes::encrypt(const buffer& iv, buffer& in)
{
    enc_.Resynchronize(&iv[0], iv.size());
    enc_.ProcessString(&in[0], in.size());
}

void aes::encrypt(const buffer& iv, std::string& in)
{
    enc_.Resynchronize(&iv[0], iv.size());
    enc_.ProcessString((byte*)&in[0], in.size());
}

void aes::decrypt(const buffer& iv, buffer& in)
{
    enc_.Resynchronize(&iv[0], iv.size());
    enc_.ProcessString(&in[0], in.size());
}

void aes::decrypt(const buffer& iv, std::string& in)
{
    enc_.Resynchronize(&iv[0], iv.size());
    enc_.ProcessString((byte*)&in[0], in.size());
}


std::string sha256(const std::string& in)
{
    std::string result;
    SHA256 hash;
    StringSource(in, true, new HashFilter(hash, new HexEncoder(new StringSink(result))));
    return result;
}

buffer sha256(const buffer& in)
{
    SHA256 hash;
    hash.Update(&in[0], in.size());
    buffer result(hash.DigestSize());
    hash.Final(&result[0]);
    return result;
}

} // namespace crypto
} // namespace hexa
