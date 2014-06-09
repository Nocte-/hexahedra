//---------------------------------------------------------------------------
/// \file   persistence_leveldb.hpp
/// \brief  Store the game data in an Sophia database.
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

#include <memory>
#include <boost/thread/mutex.hpp>
#include <boost/filesystem/path.hpp>
#include <leveldb/db.h>
#include "persistent_storage_i.hpp"

namespace hexa
{

/** Stores the terrain in an leveldb database. */
class persistence_leveldb : public persistent_storage_i
{
public:
    /** Constructor.
     * @param db_file  The database file */
    persistence_leveldb(const boost::filesystem::path& db_file
                        = "world.leveldb");

    ~persistence_leveldb();

    void store(data_type type, chunk_coordinates xyz,
               const compressed_data& data) override;
    void store(map_coordinates xy, chunk_height data) override;

    compressed_data retrieve(data_type, chunk_coordinates xyz) override;
    chunk_height retrieve(map_coordinates xy) override;

    bool is_available(data_type type, chunk_coordinates xyz) override;
    bool is_available(map_coordinates xy) override;

    void store(const es::storage& es) override;
    void store(const es::storage& es, es::storage::iterator i) override;
    void retrieve(es::storage& es) override;
    void retrieve(es::storage& es, es::entity entity_id) override;
    bool is_available(es::entity entity_id) override;

    void close();

private:
    std::unique_ptr<leveldb::DB> db_;
    leveldb::Options options_;
};

} // namespace hexa
