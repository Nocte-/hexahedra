//---------------------------------------------------------------------------
/// \file   client/gui/text_label.hpp
/// \brief  Static text label widget
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

#include "canvas.hpp"
#include "widget.hpp"

namespace hexa {
namespace gui {

class text_label : public widget
{
public:
    text_label(context& owner, const std::wstring& text, float font_size) 
       : widget (owner)
       , label_ (cnv.create_label(text, font_size))
    { } 

    void draw(canvas& cnv)
        { cnv.draw(bounding_box.first, label_); }

protected:
    std::shared_ptr<label> label_;
};

}} // namespace hexa::gui

