//---------------------------------------------------------------------------
// persistence_leveldb.cpp
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
// Copyright 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "persistence_leveldb.hpp"

#include <boost/range.hpp>
#include <boost/filesystem/operations.hpp>
#include <leveldb/filter_policy.h>
#include "log.hpp"

using namespace boost;
namespace fs = boost::filesystem;

namespace hexa {

namespace {

void check (const leveldb::Status& rc)
{
    if (!rc.ok())
        throw std::runtime_error((boost::format("persistence_leveldb: %1%") % rc.ToString()).str());
}

}

persistence_leveldb::persistence_leveldb(asio::io_service& io,
                                   const fs::path& db_file)
    : timeout_(io)
{
    options_.create_if_missing = true;
    options_.compression = leveldb::kNoCompression;
    options_.filter_policy = leveldb::NewBloomFilterPolicy(10);

    leveldb::DB* tmp;
    check(leveldb::DB::Open(options_, db_file.string(), &tmp));
    db_.reset(tmp);
}

persistence_leveldb::~persistence_leveldb()
{
    delete options_.filter_policy;
}

void persistence_leveldb::begin_transaction()
{    
    //if (txn_ == nullptr)
    //    check(mdb_txn_begin(env_, nullptr, 0, &txn_));
}

void persistence_leveldb::end_transaction()
{
    //boost::lock_guard<boost::mutex> guard (txn_lock_);
    //if (txn_ == nullptr)
    //    return;

    //check(mdb_txn_commit(txn_));
    //txn_ = nullptr;
}

void
persistence_leveldb::arm_timer()
{
    //timeout_.cancel();
    //timeout_.expires_from_now(boost::posix_time::milliseconds(500));
    //timeout_.async_wait(boost::bind(&persistence_leveldb::timeout, this,
    //                                boost::asio::placeholders::error));
    //begin_transaction();
}

void
persistence_leveldb::timeout(const boost::system::error_code& err)
{
    //if (err != boost::asio::error::operation_aborted)
    //    end_transaction();
}


//---------------------------------------------------------------------------

void
persistence_leveldb::store (data_type type, chunk_coordinates xyz,
                            const compressed_data& data)
{
    uint32_t key[4];
    key[0] = type;
    key[1] = xyz.x;
    key[2] = xyz.y;
    key[3] = xyz.z;

    auto serialized (serialize(data));
    leveldb::Slice db_key (reinterpret_cast<const char*>(key), sizeof(key));
    leveldb::Slice db_value (&*serialized.begin(), serialized.size());

    check(db_->Put(leveldb::WriteOptions(), db_key, db_value));
}

void
persistence_leveldb::store (map_coordinates xy, chunk_height z)
{
    uint32_t key[3];
    key[0] = data_type::cnk_height;
    key[1] = xy.x;
    key[2] = xy.y;

    leveldb::Slice db_key (reinterpret_cast<const char*>(key), sizeof(key));
    leveldb::Slice db_value (reinterpret_cast<const char*>(&z), sizeof(z));

    check(db_->Put(leveldb::WriteOptions(), db_key, db_value));
}

//---------------------------------------------------------------------------

compressed_data
persistence_leveldb::retrieve (data_type type, chunk_coordinates xyz)
{
    uint32_t key[4];
    key[0] = type;
    key[1] = xyz.x;
    key[2] = xyz.y;
    key[3] = xyz.z;

    leveldb::Slice db_key (reinterpret_cast<const char*>(key), sizeof(key));
    std::string result;

    check(db_->Get(leveldb::ReadOptions(), db_key, &result));

    return deserialize_as<compressed_data>(result);
}

chunk_height
persistence_leveldb::retrieve (map_coordinates xy)
{
    uint32_t key[3];
    key[0] = data_type::cnk_height;
    key[1] = xy.x;
    key[2] = xy.y;

    leveldb::Slice db_key (reinterpret_cast<const char*>(key), sizeof(key));
    std::string result;
    check(db_->Get(leveldb::ReadOptions(), db_key, &result));

    return deserialize_as<chunk_height>(result);
}

//---------------------------------------------------------------------------

bool
persistence_leveldb::is_available (data_type type, chunk_coordinates xyz)
{
    uint32_t key[4];
    key[0] = type;
    key[1] = xyz.x;
    key[2] = xyz.y;
    key[3] = xyz.z;

    leveldb::Slice db_key (reinterpret_cast<const char*>(key), sizeof(key));
    std::string result;

    auto rc (db_->Get(leveldb::ReadOptions(), db_key, &result));

    return !rc.IsNotFound();
}

bool
persistence_leveldb::is_available (data_type type, map_coordinates xy)
{
    uint32_t key[3];
    key[0] = data_type::cnk_height;
    key[1] = xy.x;
    key[2] = xy.y;

    leveldb::Slice db_key (reinterpret_cast<const char*>(key), sizeof(key));
    std::string result;

    auto rc (db_->Get(leveldb::ReadOptions(), db_key, &result));

    return !rc.IsNotFound();
}

//---------------------------------------------------------------------------

void
persistence_leveldb::store (const entity_system& es)
{
}

void
persistence_leveldb::store (const entity_system& es, es::entity entity_id)
{

}

void
persistence_leveldb::retrieve (entity_system& es)
{

}

void
persistence_leveldb::retrieve (entity_system& es, es::entity entity_id)
{

}

bool
persistence_leveldb::is_available (es::entity entity_id)
{
    return false;
}

} // namespace hexa

