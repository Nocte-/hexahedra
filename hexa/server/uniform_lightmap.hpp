//---------------------------------------------------------------------------
/// \file   server/uniform_lightmap.hpp
/// \brief  Lights every face with a fixed intensity.
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
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <hexa/basic_types.hpp>
#include <hexa/storage_i.hpp>
#include "lightmap_generator_i.hpp"

namespace hexa {

/** Light up every face with a fixed intensity. */
class uniform_lightmap : public lightmap_generator_i
{
public:
    uniform_lightmap(storage_i& cache, const boost::property_tree::ptree& conf);

    virtual ~uniform_lightmap();

    virtual lightmap& generate(const chunk_coordinates& pos,
                               const surface& s,
                               lightmap& chunk,
                               unsigned int phase = 0) const;

private:
    uint8_t sun_;
    uint8_t amb_;
    uint8_t art_;
};

} // namespace hexa

