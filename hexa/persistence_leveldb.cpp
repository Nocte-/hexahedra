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
// Copyright 2013-2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "persistence_leveldb.hpp"

#include <boost/range.hpp>
#include <boost/filesystem/operations.hpp>
#include <leveldb/filter_policy.h>
#include <leveldb/comparator.h>
#include <es/storage.hpp>

#include "log.hpp"
#include "compiler_fix.hpp"
#include "trace.hpp"

using namespace boost;
namespace fs = boost::filesystem;

namespace hexa {

namespace {

static const uint32_t type_entity = 16;


void check (const leveldb::Status& rc)
{
    if (!rc.ok())
        throw std::runtime_error((boost::format("persistence_leveldb: %1%") % rc.ToString()).str());
}

}

persistence_leveldb::persistence_leveldb(const fs::path& db_file)
{
    options_.create_if_missing = true;
    options_.filter_policy = leveldb::NewBloomFilterPolicy(10);

    leveldb::DB* tmp;
    check(leveldb::DB::Open(options_, db_file.string(), &tmp));
    db_.reset(tmp);
}

persistence_leveldb::~persistence_leveldb()
{
    db_ = nullptr;
    delete options_.filter_policy;
}

void
persistence_leveldb::close()
{
    db_ = nullptr;
    delete options_.filter_policy;
    options_.filter_policy = nullptr;
}

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

    auto ser (serialize(z));

    leveldb::Slice db_key (reinterpret_cast<const char*>(key), sizeof(key));
    leveldb::Slice db_value (&*ser.begin(), ser.size());

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
    assert(result.size() == sizeof(chunk_height));

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
persistence_leveldb::is_available (map_coordinates xy)
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
persistence_leveldb::store (const es::storage& es)
{
    std::vector<char> buffer;
    for (auto i (es.begin()); i != es.end(); ++i)
    {
        uint32_t key[2];
        key[0] = type_entity;
        key[1] = i->first;

        buffer.clear();
        es.serialize(i, buffer);

        leveldb::Slice db_key (reinterpret_cast<const char*>(key), sizeof(key));
        leveldb::Slice db_value (reinterpret_cast<const char*>(&buffer[0]), buffer.size());

        check(db_->Put(leveldb::WriteOptions(), db_key, db_value));
    }
}

void
persistence_leveldb::store (const es::storage& es, es::storage::iterator i)
{
    std::vector<char> buffer;
    uint32_t key[2];
    key[0] = type_entity;
    key[1] = i->first;

    es.serialize(i, buffer);

    leveldb::Slice db_key (reinterpret_cast<const char*>(key), sizeof(key));
    leveldb::Slice db_value (reinterpret_cast<const char*>(&buffer[0]), buffer.size());

    check(db_->Put(leveldb::WriteOptions(), db_key, db_value));
}

void
persistence_leveldb::retrieve (es::storage& es)
{
    std::unique_ptr<leveldb::Iterator> iter (db_->NewIterator(leveldb::ReadOptions()));

    uint32_t start_key[2];
    start_key[0] = type_entity;
    start_key[1] = 0;

    uint32_t end_key[2];
    end_key[0] = type_entity;
    end_key[1] = 0xffffffff;

    leveldb::Slice start (reinterpret_cast<const char*>(start_key), sizeof(start_key));
    leveldb::Slice end   (reinterpret_cast<const char*>(end_key), sizeof(end_key));

    for (iter->Seek(start);
         iter->Valid() && options_.comparator->Compare(iter->key(), end) <= 0;
         iter->Next())
    {
        const leveldb::Slice value (iter->value());
        if (value.empty())
        {
            continue;
        }
        const leveldb::Slice key (iter->key());
        const uint32_t entity (*reinterpret_cast<const uint32_t*>(key.data() + 4));

        es.deserialize(es.make(entity), { value.data(), value.data() + value.size() });
    }
}

void
persistence_leveldb::retrieve (es::storage& es, es::entity entity_id)
{

}

bool
persistence_leveldb::is_available (es::entity entity_id)
{
    return false;
}

} // namespace hexa

