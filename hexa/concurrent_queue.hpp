//---------------------------------------------------------------------------
/// \file   concurrent_queue.hpp
/// \brief  Thread-safe FIFO queue
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

#include <queue>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

namespace hexa
{

/** A thread-safe first-in, first-out queue. */
template <typename t, typename container_type = std::queue<t>>
class concurrent_queue
{
public:
    typedef t value_type;

public:
    void push(const value_type& msg)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        queue_.push_back(msg);
        cond_push_.notify_one();
    }

    void push(value_type&& msg)
    {
        boost::lock_guard<boost::mutex> lock(mutex_);

        queue_.emplace(std::move(msg));
        cond_push_.notify_one();
    }

    bool try_push(value_type&& msg)
    {
        boost::unique_lock<boost::mutex> lock(mutex_, boost::try_to_lock);
        if (!lock)
            return false;

        queue_.emplace(std::move(msg));
        cond_push_.notify_one();

        return true;
    }

    bool try_push(const value_type& msg)
    {
        boost::unique_lock<boost::mutex> lock(mutex_, boost::try_to_lock);
        if (!lock)
            return false;

        queue_.push(msg);
        cond_push_.notify_one();

        return true;
    }

    value_type pop()
    {
        boost::unique_lock<boost::mutex> lock(mutex_);

        while (queue_.empty())
            cond_push_.wait(lock);

        value_type result(std::move(queue_.front()));
        queue_.pop();

        return std::move(result);
    }

    bool try_pop(value_type& val)
    {
        boost::unique_lock<boost::mutex> lock(mutex_, boost::try_to_lock);
        if (!lock || queue_.empty())
            return false;

        std::swap(queue_.front(), val);
        queue_.pop();

        return true;
    }

    size_t size() const
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        return queue_.size();
    }

    bool empty() const
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        return queue_.empty();
    }

    void clear()
    {
        boost::lock_guard<boost::mutex> lock(mutex_);
        queue_.clear();
    }

private:
    container_type queue_;
    mutable boost::mutex mutex_;
    boost::condition_variable cond_push_;
};

} // namespace hexa
