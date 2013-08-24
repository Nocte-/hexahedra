//---------------------------------------------------------------------------
// hexa/http_cache.cpp
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
// Copyright (C) 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "http_cache.hpp"

#include <array>
#include <boost/thread.hpp>
#include <boost/filesystem/operations.hpp>
#define BOOST_NETWORK_NO_LIB
#include <boost/network/protocol/http.hpp>
#include <crypto++/hex.h>
#include <crypto++/sha.h>

#include "algorithm.hpp"

using namespace boost::network;
namespace fs = boost::filesystem;

namespace hexa {

http_cache::http_cache (const fs::path& directory)
    : dir_(directory)
{
}

fs::path http_cache::cache_file (const std::string& url) const
{
    std::array<uint8_t, 32> digest;
    CryptoPP::SHA().CalculateDigest(digest.data(),
                                    reinterpret_cast<const uint8_t*>(&url[0]),
                                    url.size());
    CryptoPP::HexEncoder hex;
    hex.Put(digest.data(), digest.size());
    std::string hexstr;
    hexstr.resize(64);
    hex.Get(reinterpret_cast<uint8_t*>(&hexstr[0]), 64);
    return dir_ / hexstr;
}

bool http_cache::is_in_cache (const std::string& url) const
{
    return fs::exists(cache_file(url));
}

void http_cache::get (const std::string& url, int timeout_msec,
                      std::function<void(std::string)> callback) const
{
    auto file (cache_file(url));
    if (fs::exists(file))
    {
        callback(file_contents(url));
        return;
    }

    boost::thread bg ([=]{
        http::client::request rq (url);
        rq << header("Connection", "close");
        http::client temp_client;
        std::string data (http::body(temp_client.get(rq)));

        std::ofstream out (file.string(), std::ios::binary);
        out.write(&data[0], data.size());
        out.close();

        callback(std::move(data));
    });
    bg.detach();
}

void http_cache::refresh (const std::string& url, int timeout_msec,
                          std::function<void(std::string)> callback) const
{

}


} // namespace hexa

