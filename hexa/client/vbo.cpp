//---------------------------------------------------------------------------
// client/vbo.cpp
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
// Copyright 2012, 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "vbo.hpp"

#include <cassert>
#include <stdexcept>
#include <boost/format.hpp>

#include <hexa/log.hpp>
#include "opengl.hpp"

using boost::format;

namespace hexa {
namespace gl {

vbo::vbo()
    : id_ (0)
    , count_ (0)
{
}

vbo::vbo(const void* buffer, size_t count, size_t vertex_size)
    : count_ (count)
{
    if (count == 0)
    {
        id_ = 0;
    }
    else
    {
        glCheck(glGenBuffers(1, &id_));
        glCheck(glBindBuffer(GL_ARRAY_BUFFER, id_));
        glCheck(glBufferData(GL_ARRAY_BUFFER, count * vertex_size,
                             buffer, GL_STATIC_DRAW));
        glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }
}

vbo& vbo::operator= (vbo&& move) noexcept
{
    if (id_)
        glCheck(glDeleteBuffers(1, &id_));

    id_ = move.id_;
    count_ = move.count_;
    move.id_ = 0;
    move.count_ = 0;

    return *this;
}

vbo::~vbo()
{
    if (id_)
        glCheck(glDeleteBuffers(1, &id_));
}

void vbo::bind() const
{
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, id_));
}

void vbo::bind_pixel_buffer() const
{
    glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, id_));
}

void vbo::unbind_pixel_buffer() const
{
    glCheck(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
}

void vbo::draw() const
{
    assert(count_ % 4 == 0);
    glCheck(glDrawArrays(GL_QUADS, 0, count_));
}

void vbo::draw_triangles() const
{
    assert(count_ % 3 == 0);
    glCheck(glDrawArrays(GL_TRIANGLES, 0, count_));
}

void vbo::unbind()
{
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

}} // namespace hexa::gl

