//---------------------------------------------------------------------------
/// \file   client/gui/context.hpp
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
// Copyright 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <rhea/simplex_solver.hpp>
#include "widget.hpp"

namespace hexa {
namespace gui {

class canvas;
class theme;

class context : boost::noncopyable, public rhea::simplex_solver
{
public:
    context(canvas& cnv, theme& thm);

    canvas& get_canvas() { return cnv_; }
    theme&  get_theme()  { return thm_; }

    void draw();

    void process_event(const event& ev);

    void add (widget::ptr&& w)
    {
        widgets_.emplace_back(std::move(w));
    }

    template <typename type>
    widget::ptr create()
    {
        return widget::ptr(new type(*this));
    }

    template <typename type>
    widget::ptr& add()
    {
        add(create<type>());
        return widgets_.back();
    }

public:
    void align_left(const widget& a, const widget& b)
    {
        add_constraint( a.left() == b.left() );
    }

    void align_right(const widget& a, const widget& b)
    {
        add_constraint( a.right() == b.right() );
    }

    void align_top(const widget& a, const widget& b)
    {
        add_constraint( a.top() == b.top() );
    }

    void align_bottom(const widget& a, const widget& b)
    {
        add_constraint( a.bottom() == b.bottom() );
    }



    void keep_inside(const widget& a, const widget& b)
    {
        add_constraints({
            a.left() >= b.left(), a.top() >= b.top(),
            a.right() <= b.right(), a.bottom() <= b.bottom() });
    }

    void overlap(const widget& a, const widget& b)
    {
        add_constraints({
            a.left() == b.left(), a.top() == b.top(),
            a.width() == b.width(), a.height() == b.height() });
    }



    void place_left_of(const widget& a, const widget& b, double padding = 0.0f)
    {
        add_constraint( a.right() + padding <= b.left() );
    }

    void place_right_of(const widget& a, const widget& b, double padding = 0.0f)
    {
        add_constraint( a.left() >= b.right() + padding );
    }

    void same_width(const widget& a, const widget& b)
    {
        add_constraint( a.width() == b.width() );
    }

    void same_height(const widget& a, const widget& b)
    {
        add_constraint( a.height() == b.height() );
    }

    void same_size(const widget& a, const widget& b)
    {
        same_width(a, b);
        same_height(a, b);
    }



    void center_h(const widget& a, const widget& b)
    {
        add_constraint( a.h_center() == b.h_center() );
    }

    void center_v(const widget& a, const widget& b)
    {
        add_constraint( a.v_center() == b.v_center() );
    }

    void center(const widget& a, const widget& b)
    {
        center_h(a, b);
        center_v(a, b);
    }

private:
    canvas&                  cnv_;
    theme&                   thm_;
    std::vector<widget::ptr> widgets_;
};

}} // namespace hexa::gui

