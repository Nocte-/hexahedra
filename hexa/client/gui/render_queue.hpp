//---------------------------------------------------------------------------
/// \file   client/gui/render_queue.hpp
/// \brief  Render queue
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
#include <memory>
#include <vector>

#include "area.hpp"
#include "types.hpp"
#include "variable.hpp"

namespace gui
{

class renderable_i : public area_i
{
public:
    virtual ~renderable_i() {}
    virtual void draw(renderer_i& target) = 0;
    virtual rect area() const = 0;
};

//---------------------------------------------------------------------------

class render_item : public renderable_i
{
public:
    render_item(const pos& p, renderer_i::texture&& img)
        : pos_{p}
        , img_{std::move(img)}
    {
    }

    void draw(renderer_i& target) override { target.draw(pos_, img_); }

    rect area() const override { return {pos_, pos_ + img_->size()}; }

    bool set_area(const rect& new_area) override
    {
        pos_ = new_area.top_left();
        return true;
    }

    virtual pos fixed_size() const { return img_->size(); }

private:
    pos pos_;
    renderer_i::texture img_;
};

//---------------------------------------------------------------------------

class render_colored_rect : public renderable_i
{
public:
    render_colored_rect(const rect& area, const color& fill)
        : area_{area}
        , fill_{fill}
    {
    }

    void draw(renderer_i& target) override { target.draw(area_, fill_); }

    rect area() const override { return area_; }

    bool set_area(const rect& new_area) override
    {
        area_ = new_area;
        return true;
    }

private:
    rect area_;
    color fill_;
};

//---------------------------------------------------------------------------

class render_textured_rect : public renderable_i
{
public:
    render_textured_rect(const rect& area, renderer_i::texture&& fill)
        : area_{area}
        , fill_{std::move(fill)}
    {
    }

    void draw(renderer_i& target) override { target.draw(area_, fill_); }

    rect area() const override { return area_; }

    bool set_area(const rect& new_area) override
    {
        area_ = new_area;
        return true;
    }

private:
    rect area_;
    renderer_i::texture fill_;
};

//---------------------------------------------------------------------------

class render_group : public renderable_i
{
public:
    typedef std::unique_ptr<renderable_i> value_type;

private:
    typedef std::vector<value_type> container_type;

public:
    typedef container_type::iterator iterator;
    typedef container_type::const_iterator const_iterator;
    typedef container_type::size_type size_type;

public:
    template <typename T, typename... Args>
    T& add(Args&&... args)
    {
        auto elem = new T{std::forward<Args>(args)...};
        items_.emplace_back(elem);
        return *elem;
    }

    void draw(renderer_i& target) override
    {
        for (auto& elem : items_)
            elem->draw(target);
    }

    rect area() const override
    {
        rect result;
        for (auto& elem : items_)
            result = make_union(result, elem->area());

        return result;
    }

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

class render_switch : public renderable_i
{
public:
    typedef render_group value_type;

public:
    render_switch(const enumeration& val)
        : val_{val}
        , groups_{val.max()}
    {
    }

    value_type& operator[](int index) { return groups_[index]; }
    const value_type& operator[](int index) const { return groups_[index]; }

    value_type& active() { return groups_[val_()]; }
    const value_type& active() const { return groups_[val_()]; }

    void draw(renderer_i& target) override { active().draw(target); }

    rect area() const override { return active().area(); }

private:
    const enumeration& val_;
    std::vector<render_group> groups_;
};

} // namespace gui
