//---------------------------------------------------------------------------
/// \file   client/gui/mouse_event_handler.hpp
/// \brief
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

#include <list>

#include <boost/signals2.hpp>
#include "area.hpp"
#include "types.hpp"
#include "variable.hpp"

namespace gui
{

class mouse_region_i : public area_i
{
public:
    virtual void move_to(const pos& p) = 0;
    virtual void button_down(int btn) = 0;
    virtual void button_up(int btn) = 0;
    virtual void wheel(int z) = 0;
    virtual void leave() = 0;
};

//---------------------------------------------------------------------------

class mouse_region : public mouse_region_i
{
public:
    boost::signals2::signal<void()> on_enter;
    boost::signals2::signal<void()> on_leave;
    boost::signals2::signal<void(const pos&)> on_move;
    boost::signals2::signal<void(const pos&)> on_drag;
    boost::signals2::signal<void(const pos&, int)> on_button_down;
    boost::signals2::signal<void(const pos&, int)> on_button_up;
    boost::signals2::signal<void(const pos&, int)> on_wheel;

public:
    mouse_region(const rect& area)
        : inside_{false}
        , dragging_{false}
        , area_{area}
    {
    }

    virtual ~mouse_region() {}

    virtual void move_to(const pos& p) override;
    virtual void button_down(int btn) override;
    virtual void button_up(int btn) override;
    virtual void wheel(int z) override;
    virtual void leave() override;

    virtual rect area() const override { return area_; }

    virtual bool set_area(const rect& r) override
    {
        area_ = r;
        return true;
    }

protected:
    bool inside_;
    bool dragging_;
    pos pos_;
    rect area_;
};

//---------------------------------------------------------------------------

class mouse_region_group : public mouse_region
{
public:
    typedef std::unique_ptr<mouse_region_i> value_type;

private:
    typedef std::list<value_type> container_type;

public:
    typedef container_type::iterator iterator;
    typedef container_type::const_iterator const_iterator;
    typedef container_type::size_type size_type;

public:
    mouse_region_group()
        : mouse_region(rect::invalid())
    {
    }

    template <typename T, typename... Args>
    T& add(Args&&... args)
    {
        auto elem = new T{std::forward<Args>(args)...};
        items_.emplace_back(elem);
        area_ = make_union(area_, elem->area());
        return *elem;
    }

    void move_to(const pos& p) override;
    void button_down(int btn) override;
    void button_up(int btn) override;
    void wheel(int z) override;
    void leave() override;

    bool empty() const { return items_.empty(); }
    size_type size() const { return items_.size(); }
    iterator begin() { return items_.begin(); }
    const_iterator begin() const { return items_.begin(); }
    iterator end() { return items_.end(); }
    const_iterator end() const { return items_.end(); }

private:
    container_type items_;
};

//---------------------------------------------------------------------------

class mouse_region_switch : public mouse_region_i
{
public:
    typedef mouse_region_group value_type;

public:
    mouse_region_switch(const enumeration& val)
        : val_{val}
        , groups_{val.max()}
    {
    }

    value_type& operator[](int index) { return groups_[index]; }
    const value_type& operator[](int index) const { return groups_[index]; }

    value_type& active() { return groups_[val_()]; }
    const value_type& active() const { return groups_[val_()]; }

    void move_to(const pos& p) override;
    void button_down(int btn) override;
    void button_up(int btn) override;
    void wheel(int z) override;
    void leave() override;

    virtual rect area() const override { return active().area(); }

private:
    const enumeration& val_;
    std::vector<value_type> groups_;
};

} // namespace gui
