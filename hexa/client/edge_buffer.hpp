//---------------------------------------------------------------------------
/// \file   client/edge_buffer.hpp
/// \brief  An OpenGL edge buffer object
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

#include <stdexcept>
#include <vector>

#include <GL/glew.h>
#include <GL/gl.h>

#include "opengl_vertex.hpp"

namespace hexa
{
namespace gl
{

/** An OpenGL edge buffer. */
template <typename t>
class edge_buffer
{
public:
    typedef t value_type;

public:
    /** Create an empty placeholder. */
    edge_buffer()
        : id_(0)
        , count_(0)
    {
    }

    /** Constructor
     * @param buffer Pointer to the edge data
     * @param count  The number of triangles in the buffer */
    edge_buffer(const void* buffer, size_t count)
        : id_(0)
        , count_(count)
    {
        if (count == 0)
            return;

        glGenBuffers(1, &id_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * 3 * sizeof(t), buffer,
                     GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        if (glGetError() != GL_NO_ERROR)
            throw std::runtime_error("vbo create failed");
    }

    template <typename tri>
    edge_buffer(const std::vector<tri>& buf)
        : edge_buffer(&buf[0], buf.size())
    {
    }

    edge_buffer(const edge_buffer&) = delete;
    edge_buffer& operator=(const edge_buffer&) = delete;

    edge_buffer(edge_buffer&& move) noexcept : id_(move.id_),
                                               count_(move.count_)
    {
        move.id_ = 0;
    }

    edge_buffer& operator=(edge_buffer&& move) noexcept
    {
        if (id_)
            glDeleteBuffers(1, &id_);

        id_ = move.id_;
        count_ = move.count_;
        move.id_ = 0;

        return *this;
    }

    ~edge_buffer()
    {
        if (id_)
            glDeleteBuffers(1, &id_);
    }

    GLuint id() const { return id_; }
    size_t triangle_count() const { return count_; }

    /** Bind this edge buffer.*/
    void bind() const { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_); }

    /** Draw the triangles.
     *  This call requires the edge buffer /and/ a vertex buffer to be
     *  bound. */
    void draw() const
    {
        glDrawElements(GL_TRIANGLES, 3 * count_, gl_type<t>()(), 0);
    }

    /** Draw a range of triangles.
     *  This call requires the edge buffer /and/ a vertex buffer to be
     *  bound. */
    void draw(size_t first, size_t count) const
    {
        assert(first + count <= count_);
        glDrawElements(GL_TRIANGLES, 3 * count, gl_type<t>()(),
                       reinterpret_cast<GLvoid*>(first * 3 * sizeof(t)));
    }

    /** Unbind this edge buffer. */
    static void unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

private:
    /** The ID of the edge buffer, as given by glGenBuffers(). */
    GLuint id_;
    /** The number of triangles in the buffer. */
    size_t count_;
};

/** Convenience function for creating a buffer from a vector of triangles. */
template <typename t>
edge_buffer<typename t::value_type> make_edge_buffer(const std::vector<t>& buf)
{
    return edge_buffer<typename t::value_type>(&buf[0], buf.size());
}

} // namespace gl
} // namespace hexa

