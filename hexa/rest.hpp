//---------------------------------------------------------------------------
/// \file   hexa/rest.hpp
/// \brief  Functions for accessing REST services
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

#include <string>
#include <functional>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>

namespace hexa
{
namespace rest
{

struct request
{
    std::string     url;
    std::string     username;
    std::string     password;
    std::string     etag;
    boost::property_tree::ptree json;


    request (const std::string& url_) : url(url_) { }

    request (const std::string& url_,
             const boost::property_tree::ptree& json_)
        : url(url_)
        , json(json_)
    { }

    request (const std::string& url_,
             boost::property_tree::ptree&& json_)
        : url(url_)
        , json(std::move(json_))
    { }

    request (const char* url_) : url(url_) { }
};

struct response
{
    int                                             status_code;
    std::unordered_map<std::string, std::string>    headers;
    std::string                                     body;
    boost::property_tree::ptree                     json;

    response() : status_code(0) { }

    bool is_ok() const { return status_code >= 200 && status_code < 300; }
};

void get(const request& req,
         std::function<void(const response&)> callback);

response get(const request& req);

void put(const request& req,
         std::function<void(const response&)> callback);

response put(const request& req);

void post(const request& req,
          std::function<void(const response&)> callback);

response post(const request& req);


} // namespace rest
} // namespace hexa
