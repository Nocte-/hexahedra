//---------------------------------------------------------------------------
/// \file   persistence_null.hpp
/// \brief  Turn off game world persistency.
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
// Copyright 2012-2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include "persistent_storage_i.hpp"

namespace hexa
{

/** This backend stores nothing. */
class persistence_null : public persistent_storage_i
{
public:
    persistence_null() {}

    void store(data_type type, chunk_coordinates xyz,
               const compressed_data& data) override
    {
    }

    void store(map_coordinates xy, chunk_height data) override {}

    compressed_data retrieve(data_type, chunk_coordinates xyz) override
    {
        return compressed_data();
    }

    chunk_height retrieve(map_coordinates xy) override
    {
        return undefined_height;
    }

    bool is_available(data_type type, chunk_coordinates xyz) override
    {
        return false;
    }

    bool is_available(map_coordinates xy) override { return false; }

    void store(const es::storage& es) override {}

    void store(const es::storage& es, es::storage::iterator entity) override {}

    void retrieve(es::storage& es) override {}

    void retrieve(es::storage& es, es::entity entity_id) override {}

    bool is_available(es::entity entity_id) override { return false; }
};

} // namespace hexa
