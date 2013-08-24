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
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include "persistent_storage_i.hpp"

namespace hexa {

/** This backend stores nothing. */
class persistence_null : public persistent_storage_i
{
public:
    persistence_null() {}

    void store (data_type type, chunk_coordinates xyz,
                const compressed_data& data) { }

    void store (map_coordinates xy, chunk_height data) { }


    compressed_data retrieve (data_type, chunk_coordinates xyz)
        { return compressed_data(); }

    chunk_height retrieve (map_coordinates xy)
        { return undefined_height; }


    bool is_available (data_type type, chunk_coordinates xyz)
        { return false; }

    bool is_available (data_type type, map_coordinates xy)
        { return false; }

    bool is_available (map_coordinates xy)
        { return false; }


    void store (const entity_system& es) { }

    void store (const entity_system& es, es::entity entity_id) { }

    void retrieve (entity_system& es) { }

    void retrieve (entity_system& es, es::entity entity_id) { }

    bool is_available (es::entity entity_id)
        { return false; }
};

} // namespace hexa

