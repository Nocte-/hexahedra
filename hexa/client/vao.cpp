//---------------------------------------------------------------------------
// client/vao.cpp
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

#include "vao.hpp"

#include <GL/gl.h>
#include <GL/glu.h>

#include <stdexcept>

namespace hexa {
namespace gl {

vao::vao()
    : vao_id_ (0)
{
}

vao::vao(const void* data, size_t count, size_t vertex_size)
    : vao_id_ (0)
    , vbo_    (data, count, vertex_size)
{
    if (count == 0)
        return;

    glGenVertexArrays(1, &vao_id_);
    assert(vao_id_ != 0);
    glBindVertexArray(vao_id_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_.id());
    glBufferData(GL_ARRAY_BUFFER, count * vertex_size, data, GL_STATIC_DRAW);
    assert(glGetError() == GL_NO_ERROR);
}

vao& vao::operator= (vao&& move)
{
    if (vao_id_)
        glDeleteVertexArrays(1, &vao_id_);

    vao_id_ = move.vao_id_;
    vbo_    = std::move(move.vbo_);
    move.vao_id_ = 0;

    return *this;
}

vao::~vao()
{
    if (vao_id_)
        glDeleteVertexArrays(1, &vao_id_);
}

void vao::draw() const
{
    assert(vao_id_);
    bind();
    glDrawArrays(GL_QUADS, 0, vertex_count());
    assert(glGetError() == GL_NO_ERROR);
}

void vao::bind() const
{
    assert(vao_id_);
    glBindVertexArray(vao_id_);
}

void vao::unbind()
{
    glBindVertexArray(0);
    vbo::unbind();
}

}} // namespace hexa::gl

