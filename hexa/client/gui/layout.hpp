//---------------------------------------------------------------------------
/// \file   hexa/client/gui/layout.hpp
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

#include <rhea/variable.hpp>
#include <rhea/simplex_solver.hpp>

#include "area.hpp"

#include <iostream>

namespace gui
{

class layout_i : public area_i
{
public:
    layout_i(const rect& init)
        : area_{init}
    { }

    virtual ~layout_i() {}
    virtual void update() = 0;

    virtual rect area() const override { return area_; }

protected:
    rect area_;
};

class layout_group : public layout_i
{
public:
    typedef std::unique_ptr<layout_i> value_type;
    typedef std::list<value_type> container_type;
    typedef container_type::iterator iterator;
    typedef container_type::const_iterator const_iterator;

public:


protected:
    container_type elements_;
};

class rhea_rect
{
public:
    rhea_rect(rhea::simplex_solver& s, area_i& dest)
        : dest_{dest}
        , left_{dest.area().left()}
        , top_{dest.area().top()}
        , width_{dest.area().width()}
        , height_{dest.area().height()}
    {
        s.add_stays({left_, top_, width_, height_});
        auto fix = dest.fixed_size();
        if (fix != pos(0,0)) {
            s.add_constraints({ width() == fix.x,
                                height() == fix.y });
        }
    }

    const rhea::variable& left() const { return left_; }
    const rhea::variable& top() const { return top_; }
    const rhea::variable& width() const { return width_; }
    const rhea::variable& height() const { return height_; }

    rhea::linear_expression right() const { return left_ + width_; }
    rhea::linear_expression bottom() const { return top_ + height_; }
    rhea::linear_expression h_center() const { return left_ + width_ / 2; }
    rhea::linear_expression v_center() const { return top_ + height_ / 2; }

    void update()
    {
        rect newrect{left_.int_value(), top_.int_value(),
                     left_.int_value() + width_.int_value(),
                     top_.int_value() + height_.int_value()};

        dest_.set_area(newrect);

        std::cout << "Update rect " << newrect.first.x << "," <<newrect.first.y
                     << " " << newrect.width() << " x " << newrect.height()
                        <<std::endl;
    }

    void suggest(rhea::simplex_solver& s, const rect& r)
    {
        s.suggest({{left_, (double)r.left()},
                         {top_, (double)r.top()},
                         {width_, (double)r.width()},
                         {height_, (double)r.height()}});
    }

    //virtual rect area() const override { return area_; }

protected:
    rect area_;
    area_i& dest_;
    rhea::variable left_, top_, width_, height_;
};

class layout_rhea : public layout_i
{
public:
    typedef rhea_rect value_type;
    typedef std::list<value_type> container_type;
    typedef container_type::iterator iterator;
    typedef container_type::const_iterator const_iterator;

public:
    layout_rhea(const rect& init)
        : layout_i{init}
        , solver_{}
        , rrect_{solver_, *this}
    {
    }

    rhea_rect& add(area_i& area)
    {
        elements_.emplace_back(solver_, area);
        return elements_.back();
    }

    void operator() (const rhea::linear_equation& constr)
    {
        solver_.add_constraint(constr);
    }

    void operator() (const rhea::linear_inequality& constr)
    {
        solver_.add_constraint(constr);
    }

    void update() override
    {
        solver_.solve();
        for (auto& elem : elements_)
            elem.update();
    }

    bool set_area(const rect& r) override
    {
        rrect_.suggest(solver_, r);
        return true;
    }

    const rhea::variable& left() const { return rrect_.left(); }
    const rhea::variable& top() const { return rrect_.top(); }
    const rhea::variable& width() const { return rrect_.width(); }
    const rhea::variable& height() const { return rrect_.height(); }

    rhea::linear_expression right() const { return rrect_.right(); }
    rhea::linear_expression bottom() const { return rrect_.bottom(); }
    rhea::linear_expression h_center() const { return rrect_.h_center(); }
    rhea::linear_expression v_center() const { return rrect_.v_center(); }

protected:
    rhea::simplex_solver solver_;
    container_type elements_;
    rhea_rect rrect_;
};

class layout_centered : public layout_i
{
public:
    typedef area_i value_type;
    typedef std::list<value_type> container_type;
    typedef container_type::iterator iterator;
    typedef container_type::const_iterator const_iterator;


};

} // namespace gui
