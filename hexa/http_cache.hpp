//---------------------------------------------------------------------------
/// \file   hexa/http_cache.hpp
/// \brief  File cache for HTTP requests.
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

#pragma once

#include <functional>
#include <string>
#include <boost/filesystem/path.hpp>

namespace hexa {

class http_cache
{
public:
    http_cache(const boost::filesystem::path& directory);

    bool is_in_cache (const std::string& url) const;

    void get (const std::string& url, int timeout_msec,
              std::function<void(std::string)> callback) const;

    void refresh (const std::string& url, int timeout_msec,
                  std::function<void(std::string)> callback) const;

private:
    boost::filesystem::path cache_file (const std::string& url) const;

private:
    boost::filesystem::path dir_;
};

} // namespace hexa

