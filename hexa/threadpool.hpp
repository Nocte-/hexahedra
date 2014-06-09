//---------------------------------------------------------------------------
/// \file   hexa/threadpool.hpp
/// \brief  Threadpool with work queue and futures.
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
// Copyright 2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include <stdexcept>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <memory>
#include <thread>
#include <vector>
#include <queue>

namespace hexa
{

/** A pool of workers and a job queue.
 *  Based on an implementation by Jakob Progsch.
 *  http://progsch.net/wordpress/?p=81
 */
class threadpool
{
public:
    /** Set up a pool with a given number of worker threads. */
    threadpool(size_t threadcount = 1)
    {
        stop_ = false;
        for (size_t i = 0; i < threadcount; ++i)
            workers_.emplace_back([=] { worker(); });
    }

    ~threadpool()
    {
        {
            std::unique_lock<std::mutex> lock{queue_mutex_};
            stop_ = true;
        }

        condition_.notify_all();
        for (auto& w : workers_)
            w.join();
    }

    /** Add a new job to the queue.
     * Example:
     * @code
     * threadpool pool;
     * std::future<int> one (pool.enqueue([]{ return 1; });
     * std::future<int> two (pool.enqueue([]{ return 2; });
     *
     * std::cout << one.get() << ", " << two.get() << std::endl;
     *
     * @param f     The function to be added to the work queue
     * @param args  Arguments for f()
     * @return An std::future object that can be used to retrieve the
     *         results of the operation.
     */
    template <typename Func, typename... Args>
    auto enqueue(Func&& f, Args&&... args)
        -> std::future<typename std::result_of<Func(Args...)>::type>
    {
        typedef typename std::result_of<Func(Args...)>::type return_type;

        if (stop_)
            throw std::runtime_error("threadpool was stopped");

        auto task =
            std::make_shared<std::packaged_task<return_type()>>(std::bind(
                std::forward<Func>(f), std::forward<Args>(args)...));

        std::future<return_type> result{task->get_future()};
        {
            std::unique_lock<std::mutex> lock{queue_mutex_};
            tasks_.push([task]() { (*task)(); });
        }
        condition_.notify_one();

        return result;
    }

private:
    void worker()
    {
        for (;;) {
            std::unique_lock<std::mutex> lock{queue_mutex_};
            while (!stop_ && tasks_.empty())
                condition_.wait(lock);

            if (stop_ && tasks_.empty())
                break;

            auto task = tasks_.front();
            tasks_.pop();
            lock.unlock();

            task();
        }
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::atomic_bool stop_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
};

/** Check if a std::future is ready. */
template <typename T>
bool is_ready(std::future<T>& f)
{
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

} // namespace hexa
