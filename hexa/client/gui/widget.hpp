//---------------------------------------------------------------------------
/// \file   client/gui/widget.hpp
/// \brief  Base class for all widgets.
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

#include <memory>
#include <boost/utility.hpp>
#include <rhea/linear_expression.hpp>
#include <rhea/variable.hpp>

#include <hexa/rectangle.hpp>
#include "../event.hpp"
#include "canvas.hpp"

namespace hexa {
namespace gui {

class context;

/** Base class for all widgets.
 *  The only thing widgets have in common is that they occupy a rectangle,
 *  and they can process events (mouse clicks, key presses, etc.).
 *  The position and size of the widget cannot be set directly; the context
 *  (the object that manages them) has a list of constraints the widgets
 *  must satisfy.  The position and size are then set by solving what is
 *  basically a list of linear equations. */
class widget : boost::noncopyable
{
    friend class context;

public:
    /** Widgets are passed around as unique pointers, so they can be kept
     ** in polymorphic containers. */
    typedef std::unique_ptr<widget> ptr;

public:
    virtual ~widget() {}

    virtual void draw(canvas& cnv) = 0;

    virtual void process_event(const event& ev);

    virtual void on_mouse_enter() { }
    virtual void on_mouse_leave() { }

public:
    const rhea::variable&   left() const   { return left_; }
    const rhea::variable&   top() const    { return top_; }
    const rhea::variable&   width() const  { return width_; }
    const rhea::variable&   height() const { return height_; }

    rhea::linear_expression right() const    { return left_ + width_; }
    rhea::linear_expression bottom() const   { return top_ + height_; }
    rhea::linear_expression h_center() const { return left_ + width_ / 2; }
    rhea::linear_expression v_center() const { return top_ + height_ / 2; }

    rectangle<float> bounding_rect() const;

protected:
    /** The constructor is protected because widgets can only be created
     ** through a context object.
     * \sa context::add
     * \sa context::make */
    widget(context& owner);

protected:
    context&        context_;
    bool            mouse_inside_flag_;
    rhea::variable  left_, top_, width_, height_;

};

}} // namespace hexa::gui

