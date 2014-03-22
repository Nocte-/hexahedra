//---------------------------------------------------------------------------
/// \file  hexa/read_write_lockable.hpp
/// \brief Single writer, multiple reader lock
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

#include <mutex>

//#include <boost/thread/locks.hpp>
//#include <boost/thread/shared_mutex.hpp>

namespace hexa {

/** Single writer, multiple reader locking. */
class readers_writer_lock
{
public:
//    typedef boost::shared_lock<boost::shared_mutex>  read_lock;
//    typedef boost::unique_lock<boost::shared_mutex>  write_lock;

    typedef std::unique_lock<std::mutex>        read_lock;
    typedef std::unique_lock<std::mutex>        write_lock;

public:
    /** Lock this object for reading.
     * Multiple readers can acquire this lock, and no writers can access
     * this object as long as a read lock is held.  The lock is released
     * once it goes out of scope. */
    read_lock  acquire_read_lock()  { return read_lock(access_);  }

    /** Lock this object for writing.
     * Only one writer can acquire this lock, and nobody else can access
     * this object as long as a write lock is held.  The lock is released
     * once it goes out of scope. */
    write_lock acquire_write_lock() { return write_lock(access_); }

private:
    //boost::shared_mutex  access_;
    std::mutex access_;
};

} // namespace hexa

