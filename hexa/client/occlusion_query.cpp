//---------------------------------------------------------------------------
// hexa/client/occlusion_query.cpp
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

#include "occlusion_query.hpp"

#include <cassert>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "opengl.hpp"
#include <hexa/log.hpp>

namespace hexa {
namespace gl {

occlusion_query::occlusion_query(bool activate)
    : id_ (0)
    , state_ (activate ? idle : inactive)
{
}

occlusion_query::~occlusion_query()
{
    if (id_ != 0)
        glCheck(glDeleteQueries(1, &id_));
}

occlusion_query& occlusion_query::operator=(occlusion_query&& move) noexcept
{
    if (&move != this)
    {
        if (id_ != 0)
            glCheck(glDeleteQueries(1, &id_));

        id_ = move.id_;
        state_ = move.state_;
        move.id_ = 0;
        move.state_ = inactive;
    }
    return *this;
}

bool occlusion_query::is_result_available() const
{
    if (id_ == 0 || state_ != busy)
        return false;

    GLuint temp (0);
    glCheck(glGetQueryObjectuiv(id_, GL_QUERY_RESULT_AVAILABLE, &temp));

    return temp == GL_TRUE;
}

unsigned int occlusion_query::result()
{
    assert(state_ != inactive);
    assert(id_ != 0);
    GLuint count (0);
    glCheck(glGetQueryObjectuiv(id_, GL_QUERY_RESULT, &count));

    return count;
}

void occlusion_query::begin_query()
{
    if (state_ == inactive)
        return;

    state_ = busy;
    if (id_ == 0)
        glCheck(glGenQueries(1, &id_));

    glCheck(glBeginQuery(GL_SAMPLES_PASSED, id_));
}

void occlusion_query::end_query() const
{
    if (state_ == inactive)
        return;

    glCheck(glEndQuery(GL_SAMPLES_PASSED));
}

void occlusion_query::dispose()
{
    if (id_ != 0)
    {
        state_ = disposed;
        glCheck(glDeleteQueries(1, &id_));
        id_ = 0;
    }
}

}} // namespace hexa::gl

