//---------------------------------------------------------------------------
/// \file   client/visibility_test.hpp
/// \brief  Can be sent to the render backend to determine chunk visibility
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
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <deque>
#include <hexa/basic_types.hpp>
#include <hexa/pos_dir.hpp>

namespace hexa {

class visibility_test : public pos_dir<chunk_coordinates>
{
public:
    enum state_t
    {
        unknown, pending_query, processing, visible, occluded
    };

    visibility_test (const chunk_coordinates& position, 
                     direction_type side,
                     state_t status_ = pending_query)
        : pos_dir (position, side), status (status_)
    { }

    bool operator== (const chunk_coordinates& compare) const
        { return pos == compare; }

    bool operator== (const visibility_test& compare) const
        { return pos == compare.pos && dir == compare.dir; }

    state_t             status;
};

typedef std::deque<visibility_test> visibility_tests;

} // namespace hexa

