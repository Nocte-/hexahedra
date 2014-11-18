//---------------------------------------------------------------------------
// hexa/rest.cpp
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

#include "rest.hpp"

#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <curl/curl.h>
#include <iostream>

#include "json.hpp"

using namespace boost::algorithm;

namespace hexa
{
namespace rest
{

namespace
{
bool is_initialized = false;
CURL* curl = nullptr;

void init_curl()
{
    if (!is_initialized) {
        is_initialized = true;
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
    }
}

size_t write_func(const char* ptr, size_t size, size_t nmemb, void* user)
{
    response& res = *reinterpret_cast<response*>(user);
    res.body.insert(res.body.end(), ptr, ptr + size * nmemb);
    return nmemb;
}

size_t header_func(const char* ptr, size_t size, size_t nmemb, void* user)
{
    response& res = *reinterpret_cast<response*>(user);

    auto bytes = size * nmemb;
    auto end = ptr + bytes;

    if (bytes > 7 && std::string(ptr, ptr + 7) == "HTTP/1.") {
        if (bytes >= 12) {
            res.headers.clear();
            res.status_code = std::stoi(std::string(ptr + 9, ptr + 12));
        } else {
            res.status_code = -1;
        }
    } else {
        auto sep = std::find(ptr, end, ':');
        if (sep != end) {
            std::string value(sep + 1, end);
            trim(value);
            res.headers[std::string(ptr, sep)] = std::move(value);
        }
    }
    return nmemb;
}

} // anonymous namespace

response get(const request& req)
{
    init_curl();
    auto curl = curl_easy_init();
    if (!curl)
        throw std::runtime_error("curl_easy_init failed");

    response res;
    struct curl_slist* headers = nullptr;

    if (req.username.empty() && req.password.empty()) {
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS,
                         CURLPROTO_HTTPS | CURLPROTO_HTTPS);
    } else {
        curl_easy_setopt(curl, CURLOPT_USERNAME, req.username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, req.password.c_str());
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    }
    curl_easy_setopt(curl, CURLOPT_URL, req.url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&res));
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_func);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, reinterpret_cast<void*>(&res));

    if (!req.etag.empty()) {
        std::string etag_hdr{"Etag :"};
        etag_hdr += req.etag;
        headers = curl_slist_append(headers, etag_hdr.c_str());
    }

    if (headers != nullptr) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    auto errcode = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (errcode != CURLE_OK)
        throw std::runtime_error(std::string("curl get failed: ")
                                 + curl_easy_strerror(errcode));

    if (starts_with(res.headers["content-type"], "application/json")) {
        std::stringstream str{res.body};
        try {
            boost::property_tree::json_parser::read_json(str, res.json);
        } catch (...) {
        }
    }

    return res;
}

void get(const request& req, std::function<void(const response&)> callback)
{
    callback(get(req));
}

response post(const request& req)
{
    init_curl();
    auto curl = curl_easy_init();

    if (!curl)
        throw std::runtime_error("curl_easy_init failed");

    response res;
    struct curl_slist* headers = nullptr;
    std::string json{to_string(req.json)};

    if (req.username.empty() && req.password.empty()) {
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS,
                         CURLPROTO_HTTPS | CURLPROTO_HTTPS);
    } else {
        curl_easy_setopt(curl, CURLOPT_USERNAME, req.username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, req.password.c_str());
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    }
    curl_easy_setopt(curl, CURLOPT_URL, req.url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)json.size());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&res));
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_func);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, reinterpret_cast<void*>(&res));

    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    auto errcode = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (errcode != CURLE_OK)
        throw std::runtime_error(std::string("curl post failed: ")
                                 + curl_easy_strerror(errcode));

    if (starts_with(res.headers["content-type"], "application/json")) {
        std::stringstream str{res.body};
        try {
            boost::property_tree::json_parser::read_json(str, res.json);
        } catch (...) {
        }
    }

    return res;
}

void post(const request& req, std::function<void(const response&)> callback)
{
    callback(post(req));
}

response put(const request& req)
{
    init_curl();
    auto curl = curl_easy_init();

    if (!curl)
        throw std::runtime_error("curl_easy_init failed");

    response res;
    struct curl_slist* headers = nullptr;
    std::string json{to_string(req.json)};

    if (req.username.empty() && req.password.empty()) {
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS,
                         CURLPROTO_HTTPS | CURLPROTO_HTTPS);
    } else {
        curl_easy_setopt(curl, CURLOPT_USERNAME, req.username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, req.password.c_str());
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    }
    curl_easy_setopt(curl, CURLOPT_URL, req.url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)json.size());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, reinterpret_cast<void*>(&res));
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_func);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, reinterpret_cast<void*>(&res));
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    auto errcode = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (errcode != CURLE_OK)
        throw std::runtime_error(std::string("curl post failed: ")
                                 + curl_easy_strerror(errcode));

    if (starts_with(res.headers["content-type"], "application/json")) {
        std::stringstream str{res.body};
        try {
            boost::property_tree::json_parser::read_json(str, res.json);
        } catch (...) {
        }
    }

    return res;
}

void put(const request& req, std::function<void(const response&)> callback)
{
    callback(put(req));
}

response del(const request& req)
{
    init_curl();
    auto curl = curl_easy_init();

    if (!curl)
        throw std::runtime_error("curl_easy_init failed");

    response res;

    if (req.username.empty() && req.password.empty()) {
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS,
                         CURLPROTO_HTTPS | CURLPROTO_HTTPS);
    } else {
        curl_easy_setopt(curl, CURLOPT_USERNAME, req.username.c_str());
        curl_easy_setopt(curl, CURLOPT_PASSWORD, req.password.c_str());
        curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    }
    curl_easy_setopt(curl, CURLOPT_URL, req.url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

    auto errcode = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (errcode != CURLE_OK)
        throw std::runtime_error(std::string("curl delete failed: ")
                                 + curl_easy_strerror(errcode));

    if (starts_with(res.headers["content-type"], "application/json")) {
        std::stringstream str{res.body};
        try {
            boost::property_tree::json_parser::read_json(str, res.json);
        } catch (...) {
        }
    }
    return res;
}

} // namespace rest
} // namespace hexa
