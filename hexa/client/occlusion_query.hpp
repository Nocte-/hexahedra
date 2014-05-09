//---------------------------------------------------------------------------
/// \file   hexa/client/occlusion_query.hpp
/// \brief  OpenGL hardware occlusion query.
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

#include <stdexcept>
#include <cstdint>
#include <hexa/compiler_fix.hpp>

namespace hexa {
namespace gl {

/** An OpenGL occlusion query. */
class occlusion_query
{
public:
    enum state_t : uint8_t
        { inactive, idle, busy, occluded, visible, cancelled, disposed };

public:
    occlusion_query(bool activate = false);

    occlusion_query(const occlusion_query&) = delete;

    occlusion_query(occlusion_query&& move) noexcept
        : id_(move.id_)
        , state_(move.state_)
    {
        move.state_ = inactive;
        move.id_ = 0;
    }

    occlusion_query& operator= (occlusion_query&& move) noexcept;

    ~occlusion_query();

    /** Check if the result of the query is already available.
     *  This should be checked before calling \a result() */
    bool is_result_available() const;

    /** Get the number of drawn pixels.
     ** If this returns zero, the object was invisible. */
    unsigned int result();

    /** All OpenGL drawing calls after this are part of the query. */
    void begin_query();

    /** End drawing, start the actual query. */
    void end_query() const;

    /** Get the OpenGL query object name. */
    unsigned int id() const { return id_; }

    operator bool () const { return state_ == idle || id_ != 0; }

    state_t state() const { return state_; }

    void cancel() { state_ = cancelled; }

    void dispose();

    void set_state(state_t s) { state_ = s; }

private:
    unsigned int id_;
    state_t      state_;
};

}} // namespace hexa::gl

