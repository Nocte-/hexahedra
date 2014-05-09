//---------------------------------------------------------------------------
/// \file   hexa/client/vbo.hpp
/// \brief  An OpenGL vertex buffer object
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
#include <vector>
#include <GL/glew.h>
#include <GL/gl.h>
#include <hexa/compiler_fix.hpp>

namespace hexa {
namespace gl {

/** An OpenGL VBO. */
class vbo
{
public:
    /** Create an empty placeholder. */
    vbo();

    /** Create a VBO.
     * @param buffer   Pointer to the vertex data.
     * @param vertex_count  The number of vertices in the buffer.
     * @param vertex_size   The size of one vertex in bytes, including
     *                      padding. */
    vbo(const void* buffer, size_t vertex_count, size_t vertex_size);

    template <typename vtx>
    vbo(const std::vector<vtx>& init)
        : vbo(&init[0], init.size(), sizeof(vtx))
    { }

    vbo(const vbo&) = delete;

    vbo(vbo&& move)
        : id_ (move.id_), count_ (move.count_)
    {
        move.id_ = 0;
        move.count_ = 0;
    }

    vbo& operator= (vbo&& move) noexcept;

    ~vbo();

    /** Get the OpenGL buffer object name. */
    GLuint id() const { return id_; }

    /** Count the number of vertices in this buffer. */
    size_t vertex_count() const { return count_; }

    /** Check if this buffer has been defined.
     *  A buffer that was created with the default constructor is
     *  undefined, and this function will return false in that case. */
    operator bool() const { return id_ != 0; }

    /** Bind this VBO.
     *  This function must be called before specifying the vertex data
     *  layout (using the bind_attributes function in opengl_vertex.hpp),
     *  and calling draw(). */
    void   bind() const;

    /** Draw this VBO as an array of quads.
     *  The VBO must have been bound first, and the vertex data layout must
     *  have been specified (using the bind_attributes function in
     *  opengl_vertex.hpp). */
    void   draw() const;

    /** Draw this VBO as an array of triangles.
     *  The VBO must have been bound first, and the vertex data layout must
     *  have been specified (using the bind_attributes function in
     *  opengl_vertex.hpp). */
    void   draw_triangles() const;

    /** Unbind this VBO. */
    static void unbind();

private:
    /** The ID of the vertex buffer, as given by glGenBuffers(). */
    GLuint id_;
    /** The number of vertices in the buffer. */
    size_t count_;
};

/** Convenience function for creating a VBO from a vector of vertices. */
template <typename t>
vbo make_vbo(const std::vector<t>& buf)
{
    return vbo(&buf[0], buf.size(), sizeof(t));
}

}} // namespace hexa::gl

