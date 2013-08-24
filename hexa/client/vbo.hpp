//---------------------------------------------------------------------------
/// \file   client/vbo.hpp
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
// Copyright 2012, 2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

#include <stdexcept>
#include <vector>
#include <boost/utility.hpp>
#include <GL/glew.h>
#include <GL/gl.h>

namespace hexa {
namespace gl {

/** An OpenGL VBO. */
class vbo : boost::noncopyable
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

    /// \todo Figure out what's up with gcc 4.7 demanding a copy constructor
    vbo(const vbo&)
    {
        throw std::runtime_error("vbo cannot be copied");
    }

    vbo(vbo&& move) noexcept
        : id_ (move.id_), count_ (move.count_)
    {
        move.id_ = 0;
    }

    vbo& operator= (vbo&& move) noexcept;

    ~vbo();

    GLuint id() const { return id_; }
    size_t vertex_count() const { return count_; }

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

