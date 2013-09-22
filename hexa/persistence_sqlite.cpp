//---------------------------------------------------------------------------
// persistence_sqlite.cpp
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
// Copyright 2012, 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "persistence_sqlite.hpp"

#include <boost/range.hpp>
#include <sqlite3.h>

using namespace boost;
namespace fs = boost::filesystem;

namespace hexa {

std::string err_str_(int rc)
{
#if SQLITE_VERSION_NUMBER < 3007015
    return std::to_string(rc);
#else
    return sqlite3_errstr(rc);
#endif
}

persistence_sqlite::persistence_sqlite(asio::io_service& io,
                                       const fs::path& db_file,
                                       const fs::path& setup)
    : io_ (io)
    , timeout_ (io)
    , db_ (db_file, setup)
{
    read_.emplace_back  (db_.prepare_statement("SELECT data FROM area WHERE x=? AND y=? AND idx=?"));
    write_.emplace_back (db_.prepare_statement("INSERT OR REPLACE INTO area (x,y,idx,data) VALUES (?,?,?,?)"));
    exists_.emplace_back(db_.prepare_statement("SELECT count(*) FROM area WHERE x=? AND y=? AND idx=? LIMIT 1"));

    read_.emplace_back  (db_.prepare_statement("SELECT data FROM chunk WHERE x=? AND y=? AND z=?"));
    write_.emplace_back (db_.prepare_statement("INSERT OR REPLACE INTO chunk (x,y,z,data) VALUES (?,?,?,?)"));
    exists_.emplace_back(db_.prepare_statement("SELECT count(*) FROM chunk WHERE x=? AND y=? AND z=? LIMIT 1"));

    read_.emplace_back  (db_.prepare_statement("SELECT data FROM surface WHERE x=? AND y=? AND z=?"));
    write_.emplace_back (db_.prepare_statement("INSERT OR REPLACE INTO surface (x,y,z,data) VALUES (?,?,?,?)"));
    exists_.emplace_back(db_.prepare_statement("SELECT count(*) FROM surface WHERE x=? AND y=? AND z=? LIMIT 1"));

    read_.emplace_back  (db_.prepare_statement("SELECT data FROM lightmap WHERE x=? AND y=? AND z=?"));
    write_.emplace_back (db_.prepare_statement("INSERT OR REPLACE INTO lightmap (x,y,z,data) VALUES (?,?,?,?)"));
    exists_.emplace_back(db_.prepare_statement("SELECT count(*) FROM lightmap WHERE x=? AND y=? AND z=? LIMIT 1"));

    read_height_ = db_.prepare_statement("SELECT z FROM height WHERE x=? AND y=?");
    write_height_= db_.prepare_statement("INSERT OR REPLACE INTO height (x,y,z) VALUES (?,?,?)");
    exists_.emplace_back(db_.prepare_statement("SELECT count(*) FROM height WHERE x=? AND y=? LIMIT 1"));

    db_.exec("PRAGMA synchronous=OFF;");
}

persistence_sqlite::~persistence_sqlite()
{
    end_transaction();
}

void persistence_sqlite::begin_transaction()
{
    boost::mutex::scoped_lock tr_lock (transaction_lock_);
    if (!transaction_)
        transaction_.reset(new sql::transaction(db_.begin_transaction()));
}

void persistence_sqlite::end_transaction()
{
    boost::mutex::scoped_lock tr_lock (transaction_lock_);
    if (transaction_)
    {
        transaction_->commit();
        transaction_ = nullptr;
    }
}

void
persistence_sqlite::arm_timer()
{
    timeout_.cancel();
    timeout_.expires_from_now(boost::posix_time::milliseconds(500));
    timeout_.async_wait(boost::bind(&persistence_sqlite::timeout, this,
                                    boost::asio::placeholders::error));
    begin_transaction();
}

void
persistence_sqlite::timeout(const boost::system::error_code& err)
{
    if (err != boost::asio::error::operation_aborted)
        end_transaction();
}


//---------------------------------------------------------------------------

void
persistence_sqlite::store (data_type type, chunk_coordinates xyz,
                           const compressed_data& data)
{
    boost::mutex::scoped_lock l (lock);

    unsigned int idx (static_cast<unsigned int>(type));
    assert(idx < write_.size());
    auto& query (write_[idx]);

    arm_timer();
    query.reset();

    query.bind(1, xyz.x);
    query.bind(2, xyz.y);
    query.bind(3, xyz.z);
    query.bind(4, serialize(data));

    auto rc (query.step());
    if (rc != SQLITE_DONE && rc != SQLITE_OK)
        throw std::runtime_error(std::string("cannot store data : ") + err_str_(rc));
}

void
persistence_sqlite::store (map_coordinates xy, chunk_height z)
{
    boost::mutex::scoped_lock l (lock);

    auto& query (write_height_);

    arm_timer();
    query.reset();

    query.bind(1, xy.x);
    query.bind(2, xy.y);
    query.bind(3, z);

    auto rc (query.step());
    if (rc != SQLITE_DONE && rc != SQLITE_OK)
        throw std::runtime_error(std::string("cannot store data : ") + err_str_(rc));
}

//---------------------------------------------------------------------------

compressed_data
persistence_sqlite::retrieve (data_type type, chunk_coordinates xyz)
{
    boost::mutex::scoped_lock l (lock);

    int idx (static_cast<int>(type));
    auto& query (read_[idx]);

    arm_timer();
    query.reset();

    query.bind(1, xyz.x);
    query.bind(2, xyz.y);
    query.bind(3, xyz.z);

    auto rc (query.step());
    if (rc != SQLITE_DONE && rc != SQLITE_ROW)
    {
        std::stringstream msg;
        msg << "data type " << (int)type << " at " << xyz << ": " << err_str_(rc);
        throw not_in_storage_error(msg.str());
    }

    return deserialize_as<compressed_data>(query.get_blob(0));
}

chunk_height
persistence_sqlite::retrieve (map_coordinates xy)
{
    boost::mutex::scoped_lock l (lock);

    auto& query (read_height_);

    arm_timer();
    query.reset();

    query.bind(1, xy.x);
    query.bind(2, xy.y);

    auto rc (query.step());
    if (rc != SQLITE_DONE && rc != SQLITE_ROW)
    {
        std::stringstream msg;
        msg << "coarse map height at " << xy << ": " << err_str_(rc);
        throw not_in_storage_error(msg.str());
    }

    return query.get_uint(0);
}

//---------------------------------------------------------------------------

bool
persistence_sqlite::is_available (data_type type, chunk_coordinates xyz)
{
    boost::mutex::scoped_lock l (lock);

    int idx (static_cast<int>(type));
    auto& query (exists_[idx]);

    query.reset();

    query.bind(1, xyz.x);
    query.bind(2, xyz.y);
    query.bind(3, xyz.z);

    int rc (query.step());
    if (rc != SQLITE_DONE && rc != SQLITE_ROW)
    {
        assert(false); // Not unrecoverable, but shouldn't happen.
        return false;
    }

    return query.get_int() > 0;
}

bool
persistence_sqlite::is_available (data_type type, map_coordinates xy)
{
    boost::mutex::scoped_lock l (lock);

    int idx (static_cast<int>(type));
    auto& query (exists_[idx]);

    query.reset();

    query.bind(1, xy.x);
    query.bind(2, xy.y);

    int rc (query.step());
    if (rc != SQLITE_DONE && rc != SQLITE_ROW)
    {
        assert(false); // Not unrecoverable, but shouldn't happen.
        return false;
    }

    return query.get_int() > 0;
}

//---------------------------------------------------------------------------

void
persistence_sqlite::store (const entity_system& es)
{
}

void
persistence_sqlite::store (const entity_system& es, es::entity entity_id)
{

}

void
persistence_sqlite::retrieve (entity_system& es)
{

}

void
persistence_sqlite::retrieve (entity_system& es, es::entity entity_id)
{

}

bool
persistence_sqlite::is_available (es::entity entity_id)
{
    return false;
}

} // namespace hexa

