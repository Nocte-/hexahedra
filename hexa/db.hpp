//---------------------------------------------------------------------------
/// \file   hexa/db.hpp
/// \brief  Wrapper classes for Sqlite
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
// Copyright 2012-2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/filesystem/path.hpp>
#include <sqlite3.h>

namespace hexa {
namespace sql {

/** A blob is binary data of any size. */
typedef std::vector<char> blob;

/** This exception is thrown by the sql classes. */
class error : public std::runtime_error
{
    int code_;
public:
    error(int code, const char* what)
        : std::runtime_error(what)
        , code_ (code)
    { }

    virtual ~error() throw() {}

    /** Get the extended error code as reported by the database driver. */
    int code() const throw() { return code_; }
};

//---------------------------------------------------------------------------

class db;

/** SQL statements can be precompiled into prepared statements for faster
 ** execution.
 * A prepared statement usually contains one or more placeholders (written
 * as '?' in the SQL statement) that need to be bound to a variable before
 * it can be used.  This is done using the bind() functions. */
class prepared_statement : boost::noncopyable
{
    sqlite3_stmt* st_;

protected:
    friend class db;

    /** Constructor is protected: only the database itself can create
     ** a prepared statement object. */
    prepared_statement(sqlite3_stmt* statement);

public:
    prepared_statement();
    ~prepared_statement();

    prepared_statement(prepared_statement&& move);
    prepared_statement& operator=(prepared_statement&& move);

    /** Bind an integer value to a named parameter. */
    void bind (const std::string& param_name, int value);
    void bind (const std::string& param_name, unsigned int value);
    void bind (const std::string& param_name, double value);
    void bind (const std::string& param_name, const std::string& value);
    void bind (const std::string& param_name, const blob& value);

    /** Bind an integer value to a parameter by its index. */
    void bind (int param_index, int value);
    void bind (int param_index, unsigned int value);
    void bind (int param_index, double value);
    void bind (int param_index, const std::string& value);
    void bind (int param_index, const blob& value);

    /** Reset the prepared statement completely. */
    void reset();

    /** Remove all the variable bindings. */
    void clear_bindings();

    /** Execute a single step of this statement.
     *  Afterward, the results can be queried with get_int(), get_uint(),
     *  get_double(), etc.
     *  @return An sqlite error code */
    int  step();

    /** Fetch an integer from the results.
     *  This should be called after step(). */
    int         get_int    (int column = 0) const;

    /** Fetch an unsigned integer from the results.
     *  This should be called after step(). */
    unsigned    get_uint   (int column = 0) const;

    /** Fetch a double from the results.
     *  This should be called after step(). */
    double      get_double (int column = 0) const;

    /** Fetch a string from the results.
     *  This should be called after step(). */
    std::string get_text   (int column = 0) const;

    /** Fetch some binary data from the results.
     *  This should be called after step(). */
    blob        get_blob   (int column = 0) const;

private:
    int param_index (const std::string& param_name) const;
};

/** RAII class for database transactions.
 *  The transaction starts when this object is constructed, and ends when
 *  commit() is called.  If the object gets destroyed before the transaction
 *  is committed, it rolls back the transaction. */
class transaction : boost::noncopyable
{
    sqlite3* pdb_;
    bool     commit_;
    bool     rollback_;

    friend class db;

protected:
    /** Constructor is protected: only the database itself can create
     ** a transaction object. */
    transaction (sqlite3* db);

public:
    ~transaction();
    transaction(transaction&& move);

    /** Commit the transaction. */
    void commit();
    /** Roll back the transaction. */
    void rollback();
};

//---------------------------------------------------------------------------

/** Wrapper class for an sqlite3 database.
 *  Example:
 *  @code

 sql::db my_db ("foo.db", "setup.sql");
 auto new_item (my_db.prepare_statement("INSERT INTO fruit VALUES (?, ?, ?)"));

 // Put some values in the '?' places of the SQL query
 new_item.bind(0, std::string("banana"));
 new_item.bind(1, 1.99);
 new_item.bind(2, std::string("yellow"));

 // Execute the prepared statement
 new_item.step();

 *  @endcode */
class db : boost::noncopyable
{
    sqlite3* pdb_; /**< The sqlite database handle */

public:
    /** Open a database.
     * If the database was not found, a new one is created using the statements
     * in the setup file.
     * @param db_filename   The file name of the sqlite3 database
     * @param setup_file    The file with the SQL statements that need to
     *                      be executed to set up a new database */
    db(const boost::filesystem::path& db_filename,
       const boost::filesystem::path& setup_file = "");

    ~db();

    /** Execute an SQL statement.
     * @param sql  The SQL statement. */
    void exec(const std::string& sql) const;

    /** Create a prepared statement.
     *  It can be executed at a later point using prepared_statement::bind()
     *  and prepared_statement::step().
     * @param sql   The SQL statement
     * @return The prepared statement that was compiled from \a sql */
    prepared_statement prepare_statement(const std::string& sql) const;

    /** Create an online backup of the current database.
     * This happens in the background.  Any changes to the DB that are
     * done during the backup are also backed up.
     * @param backup_file   The backup DB
     * @param progress      Callback for a progress indicator.  The first
     *               parameter is the current step, the second parameter
     *               the total number of steps. */
    void make_online_backup(const std::string& backup_file,
                            std::function<void (int, int)> progress);

    /** Begin a transaction.
     * Note that you must call commit() on the transaction object to
     * finish it.  Failure to do so will result in a rollback once the
     * object goes out of scope.
     * @return A RAII transaction object */
    transaction begin_transaction() const;

protected:
    /** Check if the previous operation completed successfully.
     *  If this is not the case, throw an exception.
     * @throw error
     * @param error_code  The return code of an sqlite3 call */
    void check(int error_code) const;
};

}} //namespace hexa::sql

