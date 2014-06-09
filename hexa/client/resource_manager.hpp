//---------------------------------------------------------------------------
/// \file   client/resource_manager.hpp
/// \brief  Base class for loading and caching resources.
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

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/thread/mutex.hpp>
#include <boost/filesystem/path.hpp>

namespace hexa
{

enum resource_type {
    res_block_texture,
    res_model,
    res_texture,
    res_tile,
    res_image,
    res_icon,
    res_sound_fx,
    res_music,
    res_ui,
    res_font,
    res_shader
};

boost::filesystem::path resource_path(resource_type type);

std::vector<std::string> resource_extensions(resource_type type);

boost::filesystem::path resource_file(resource_type type,
                                      const std::string& name);

/** Base class for resource managers. */
template <class type>
class resource_manager
{
public:
    typedef type value_type;

    /** Resources are passed around as shared pointers.
     *  The reference counting is also used in cleanup(), to make sure no
     *  resources get unloaded that are still in use by some part of the
     *  application. */
    typedef std::shared_ptr<value_type> resource;

public:
    /** Get a resource by its name, load it if necessary.
     * @param name  The resource's name
     * @return A shared pointer to the resource, or nullptr if it could
     *         not be found. */
    resource operator()(const std::string& name)
    {
        boost::mutex::scoped_lock lock(mutex_);

        auto find(resources_.find(name));
        if (find == resources_.end()) {
            auto loaded(load(name));
            find = resources_.insert(std::make_pair(name, loaded)).first;
        }

        return find->second;
    }

    /** Map a function to every loaded resource.
     *  The function should take two parameters: the resource's name,
     *  and the shared pointer. */
    void for_each(std::function<void(std::string, resource)> func)
    {
        boost::mutex::scoped_lock lock(mutex_);

        for (auto p : resources_) {
            if (p.second != nullptr)
                func(p.first, p.second);
        }
    }

    /** Unload all resources that are no longer referenced. */
    void cleanup()
    {
        boost::mutex::scoped_lock lock(mutex_);

        for (auto i(resources_.begin()); i != resources_.end();) {
            auto j(i++);
            if (j->second.unique())
                resources_.erase(j);
        }
    }

protected:
    virtual resource load(const std::string& location) = 0;

    virtual void unload(const std::string& location) {}

private:
    boost::mutex mutex_;
    std::unordered_map<std::string, resource> resources_;
};

} // namespace hexa
