//---------------------------------------------------------------------------
/// \file  lru_cache.hpp
/// \brief Associative container for in a Least Recently Used cache.
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
// Copyright 2012-2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include <cstddef>
#include <list>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <boost/optional.hpp>

namespace hexa
{

/** Associative container for in a Least Recently Used cache.
 * Example:
 * @code

hexa::lru_cache<int, std::string>  cache;

cache[1] = "one";
cache[8] = "eight";
cache[5] = "five";

// Prune the cache down to the last two used elements.
cache.prune(2);

std::cout << cache.count(8) << " " << cache.count(0) << std::endl;
// Output: "1 0"

// Add a new element.
cache[10] = "ten";

// The oldest element is now '8'.  Make that one the newest.
cache.touch(8);

// Kick '5' out of the cache.
cache.prune(2);

// 'for_each' accesses all elements, newest to oldest.
cache.for_each([](int k, std::string v){ std::cout << k << " -> " << v <<
std::endl; });
// Prints:
// 8 -> eight
// 10 -> ten

* @endcode */
template <typename Key, typename Value>
class lru_cache
{
    typedef std::pair<Key, Value> pair_t;
    typedef std::list<pair_t> list_t;
    typedef std::unordered_map<Key, typename list_t::iterator> map_t;

    list_t list_;
    map_t map_;
    size_t size_;

public:
    typedef Key key_type;
    typedef Value mapped_type;
    typedef pair_t value_type;

    typedef typename list_t::const_iterator const_iterator;

public:
    /** Construct an empty cache. */
    lru_cache()
        : size_{0}
    {
    }

    /** Prune the cache back to a given size.
     *  If the cache is larger than the maximum, the oldest entries
     *  will be deleted.
     * @post size() <= max_size
     * @param max_size  The maximum cache size */
    void prune(size_t max_size)
    {
        while (size_ > max_size) {
            map_.erase(list_.back().first);
            list_.pop_back();
            --size_;
        }
    }

    /** Prune the cache back to a given size, deleting only items that
     ** satisfy a given condition.
     *  If the cache is larger than the maximum, the oldest entries
     *  will be deleted.  Unlike prune(), there is no guarantee the
     *  cache will be smaller than max_size after the function returns.
     * @param max_size  The maximum cache size
     * @param op        Only elements for which op(x) is true are pruned */
    template <typename Pred>
    void prune_if(size_t max_size, Pred op)
    {
        if (list_.empty())
            return;

        auto i = std::prev(list_.end());
        while (size_ > max_size) {
            if (op(*i)) {
                map_.erase(i->first);
                i = list_.erase(i);
                --size_;
            }
            if (i == list_.begin())
                return;

            --i;
        }
    }

    /** Prune the cache back to a given size.
     *  If the cache is larger than the maximum, the oldest entries
     *  will be deleted.  This version of prune() will invoke a callback
     *  for every element that is removed.
     * @post size() <= max_size
     * @param max_size  The maximum cache size
     * @param on_remove Callback for removed elements */
    template <typename Func>
    void prune(size_t max_size, Func on_remove)
    {
        while (size_ > max_size) {
            pair_t& tbr = list_.back();
            on_remove(tbr.first, tbr.second);
            map_.erase(tbr.first);
            list_.pop_back();
            --size_;
        }
    }

    /** Empty the cache. */
    void clear()
    {
        map_.clear();
        list_.clear();
        size_ = 0;
    }

    /** Mark an element as recently used. */
    void touch(const key_type& k)
    {
        auto found = map_.find(k);
        assert(found != map_.end());
        if (found != map_.end())
            list_.splice(list_.begin(), list_, found->second);
    }

    /** Fetch an element from the cache.
     *  If the key does not exist yet, a new empty element will be
     *  created. */
    mapped_type& operator[](const key_type& k)
    {
        auto found = map_.find(k);
        if (found == map_.end()) {
            list_.push_front(pair_t(k, mapped_type()));
            map_[k] = list_.begin();
            ++size_;

            return list_.begin()->second;
        }
        list_.splice(list_.begin(), list_, found->second);

        return found->second->second;
    }

    /** Remove an element from the cache. */
    void remove(const key_type& k)
    {
        auto found = map_.find(k);
        if (found == map_.end())
            return;

        list_.erase(found->second);
        map_.erase(found);
        --size_;
    }

    /** Count the number of elements for a given key.
     *  The returned value is always 0 or 1. */
    size_t count(const key_type& k) const { return map_.count(k); }

    /** Get the number of elements in the cache. */
    size_t size() const { return size_; }

    /** Check if the cache is empty. */
    bool empty() const { return size_ == 0; }

    /** Get an element from the cache without changing its age. */
    mapped_type& get(const key_type& k) const
    {
        auto found = map_.find(k);
        if (found == map_.end())
            throw std::runtime_error("lru_cache::get: key not found");

        return found->second->second;
    }

    boost::optional<mapped_type&> try_get(const key_type& k) const
    {
        auto found = map_.find(k);
        if (found == map_.end())
            return boost::optional<mapped_type&>();

        return found->second->second;
    }

    boost::optional<mapped_type&> try_get(const key_type& k)
    {
        auto found = map_.find(k);
        if (found == map_.end())
            return boost::optional<mapped_type&>();

        list_.splice(list_.begin(), list_, found->second);
        return found->second->second;
    }

    /** Call a function for every key-value pair in the cache. */
    template <typename Func>
    Func for_each(Func op) const
    {
        for (const pair_t& p : list_)
            op(p.first, p.second);

        return op;
    }

    const_iterator begin() const { return std::begin(list_); }
    const_iterator end() const { return std::end(list_); }
};

} // namespace hexa
