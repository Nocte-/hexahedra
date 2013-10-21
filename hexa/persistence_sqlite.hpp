//---------------------------------------------------------------------------
/// \file   persistence_sqlite.hpp
/// \brief  Store the terrain in an sqlite database.
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

#include <memory>
#include <boost/asio.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/filesystem/path.hpp>
#include "db.hpp"
#include "persistent_storage_i.hpp"

namespace hexa {

/** Stores the terrain in an sqlite database. */
class persistence_sqlite : public persistent_storage_i
{
public:
    /** Constructor.
     * \param io       The boost asio service
     * \param db_file  The database file
     * \param setup    If the database does not exist yet, it is initialized
     *                 using this SQL script */
    persistence_sqlite(boost::asio::io_service& io,
                       const boost::filesystem::path& db_file = "world.db",
                       const boost::filesystem::path& setup   = "dbsetup.sql");

    ~persistence_sqlite();

    void store (data_type type, chunk_coordinates xyz, const compressed_data& data);
    void store (map_coordinates xy, chunk_height data);

    compressed_data retrieve (data_type, chunk_coordinates xyz);
    chunk_height    retrieve (map_coordinates xy);

    bool is_available (data_type type, chunk_coordinates xyz);
    bool is_available (data_type type, map_coordinates xy);


    void store (const entity_system& es);
    void store (const entity_system& es, es::entity entity_id);
    void retrieve (entity_system& es);
    void retrieve (entity_system& es, es::entity entity_id);
    bool is_available (es::entity entity_id);

protected:
    void  begin_transaction();
    void  end_transaction();

private:
    void  arm_timer();
    void  timeout(const boost::system::error_code& err);

private:
    boost::asio::deadline_timer timeout_;

    sql::db db_;
    std::unique_ptr<sql::transaction>   transaction_;
    boost::mutex                        transaction_lock_;

    std::vector<sql::prepared_statement>  read_;
    std::vector<sql::prepared_statement>  write_;
    std::vector<sql::prepared_statement>  exists_;

    sql::prepared_statement  read_height_;
    sql::prepared_statement  write_height_;
    sql::prepared_statement  exists_height_;
};

} // namespace hexa

