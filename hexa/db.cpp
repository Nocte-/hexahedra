//---------------------------------------------------------------------------
// server/db.cpp
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
// Copyright 2011, 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "db.hpp"

#include <cassert>
#include <boost/format.hpp>
#include <boost/filesystem/convenience.hpp>
#include <iostream>
#include <fstream>

namespace fs = boost::filesystem;

namespace hexa {
namespace sql {

prepared_statement::prepared_statement()
    : st_ (nullptr)
{
}

prepared_statement::prepared_statement(sqlite3_stmt* statement)
    : st_ (statement)
{
}

prepared_statement::~prepared_statement()
{
    if (st_)
        sqlite3_finalize(st_);
}

prepared_statement::prepared_statement(prepared_statement&& move)
    : st_ (move.st_)
{
    move.st_ = nullptr;
}

prepared_statement& prepared_statement::operator=(prepared_statement&& move)
{
    if (st_ != move.st_)
    {
        if (st_)
            sqlite3_finalize(st_);

        st_ = move.st_;
        move.st_ = nullptr;
    }
    return *this;
}

int prepared_statement::param_index (const std::string& name) const
{
    assert(st_);
    int index (sqlite3_bind_parameter_index(st_, name.c_str()));

    if (index == 0)
        throw error(0, (boost::format("cannot bind parameter '%s'") % name).str().c_str());

    return index;
}


void prepared_statement::bind (const std::string& param_name, int value)
{
    bind(param_index(param_name), value);
}

void prepared_statement::bind (const std::string& param_name, unsigned value)
{
    bind(param_index(param_name), value);
}

void prepared_statement::bind (const std::string& param_name, double value)
{
    bind(param_index(param_name), value);
}

void prepared_statement::bind (const std::string& param_name,
                               const std::string& value)
{
    bind(param_index(param_name), value);
}

void prepared_statement::bind (const std::string& param_name,
                               const blob& value)
{
    bind(param_index(param_name), value);
}


void prepared_statement::bind (int index, int value)
{
    assert(st_);
    sqlite3_bind_int(st_, index, value);
}

void prepared_statement::bind (int index, unsigned int value)
{
    assert(st_);
    sqlite3_bind_int(st_, index, value);
}

void prepared_statement::bind (int index, double value)
{
    assert(st_);
    sqlite3_bind_double(st_, index, value);
}

void prepared_statement::bind (int index, const std::string& value)
{
    assert(st_);
    sqlite3_bind_text(st_, index, value.c_str(), value.size(),
                      SQLITE_TRANSIENT);
}

void prepared_statement::bind (int index, const blob& value)
{
    assert(st_);
    if (value.empty())
    {
        sqlite3_bind_blob(st_, index, 0, 0, SQLITE_STATIC);
    }
    else
    {
        sqlite3_bind_blob(st_, index, &*value.begin(), value.size(),
                          SQLITE_TRANSIENT);
    }
}

void prepared_statement::reset()
{
    assert(st_);
    sqlite3_reset(st_);
}

void prepared_statement::clear_bindings()
{
    assert(st_);
    sqlite3_clear_bindings(st_);
}

int prepared_statement::step()
{
    assert(st_);
    return sqlite3_step(st_);
}

int prepared_statement::get_int(int col) const
{
    assert(st_);
    return sqlite3_column_int(st_, col);
}

unsigned int prepared_statement::get_uint(int col) const
{
    assert(st_);
    return sqlite3_column_int(st_, col);
}

double prepared_statement::get_double(int col) const
{
    assert(st_);
    return sqlite3_column_double(st_, col);
}

std::string prepared_statement::get_text(int col) const
{
    assert(st_);
    size_t len (sqlite3_column_bytes(st_, col));
    return std::move(std::string((char*)sqlite3_column_text(st_, col), len));
}

blob prepared_statement::get_blob(int col) const
{
    assert(st_);
    size_t len (sqlite3_column_bytes(st_, col));
    auto first (reinterpret_cast<const char*>(sqlite3_column_blob(st_, col)));
    return std::move(blob(first, first + len));
}

//---------------------------------------------------------------------------

transaction::transaction(sqlite3* db)
    : pdb_ (db)
    , commit_ (false)
{
    sqlite3_exec(pdb_, "BEGIN;", 0, 0, 0);
}

transaction::transaction(transaction&& move)
    : pdb_ (move.pdb_)
    , commit_ (move.commit_)
{
    move.commit_ = true;
}

transaction::~transaction()
{
    if (!commit_)
    {
        try
        {
            rollback();
        }
        catch(...)
        {
            // ...
        }
    }
}

void transaction::commit()
{
    assert(!commit_);
    if (!commit_)
    {
        sqlite3_exec(pdb_, "COMMIT;", 0, 0, 0);
        commit_ = true;
    }
}

void transaction::rollback()
{
    assert(!commit_);
    if (!commit_)
    {
        sqlite3_exec(pdb_, "ROLLBACK;", 0, 0, 0);
        commit_ = true;
    }
}

//---------------------------------------------------------------------------

db::db(const fs::path& db_filename, const fs::path& init_filename)
    : pdb_(nullptr)
{
    if (!sqlite3_threadsafe())
        throw std::runtime_error("sqlite is not threadsafe");

    bool db_existed (fs::exists(db_filename));

    check(sqlite3_open_v2(db_filename.string().c_str(), &pdb_, 
                            SQLITE_OPEN_READWRITE 
                          | SQLITE_OPEN_CREATE 
                          | SQLITE_OPEN_FULLMUTEX, nullptr));

    check(sqlite3_extended_result_codes(pdb_, 1));

    if (!db_existed && !init_filename.empty())
    {
        // Populate the empty DB first.
        std::ifstream statements (init_filename.string().c_str());
        if (statements)
        {
            std::string statement;
            while (std::getline(statements, statement))
                exec(statement);
        }
        else
        {
            throw std::runtime_error((boost::format("Database %1% does not exist, and cannot read %2% to populate it") % db_filename.string() % init_filename.string()).str());
        }
    }
}

db::~db()
{
    sqlite3_close(pdb_);
}

void db::check(int error_code) const
{
    if (error_code != SQLITE_OK)
        throw error(sqlite3_extended_errcode(pdb_), sqlite3_errmsg(pdb_));
}

void db::exec(const std::string& sql) const
{
    check(sqlite3_exec(pdb_, sql.c_str(), 0, 0, 0));
}

prepared_statement db::prepare_statement(const std::string& sql) const
{
    const char* dummy;
    sqlite3_stmt* st;
    check(sqlite3_prepare_v2(pdb_, sql.c_str(), sql.size(), &st, &dummy));

    return std::move(prepared_statement(st));
}

void db::make_online_backup(const std::string& backup_filename,
                            std::function<void (int, int)> progress)
{
    struct backup_raii
    {
        db              db_;
        sqlite3_backup* h_;

        backup_raii(const std::string& file, db& src)
            : db_(file)
            , h_ (sqlite3_backup_init(db_.pdb_, "main", src.pdb_, "main"))
        { if (!h_) throw error(0, sqlite3_errmsg(db_.pdb_)); }

        ~backup_raii() { sqlite3_backup_finish(h_); }
    };


    backup_raii bak (backup_filename, *this);

    for (;;)
    {
        int rc (sqlite3_backup_step(bak.h_, 8));

        progress(sqlite3_backup_remaining(bak.h_),
                 sqlite3_backup_pagecount(bak.h_));

        if (rc == SQLITE_OK || rc == SQLITE_BUSY || rc == SQLITE_LOCKED)
            sqlite3_sleep(250);
        else if (rc == SQLITE_DONE)
            break;
        else
            throw error(rc, sqlite3_errmsg(pdb_));
    }
}

transaction db::begin_transaction() const
{
    return transaction(pdb_);
}

}}
