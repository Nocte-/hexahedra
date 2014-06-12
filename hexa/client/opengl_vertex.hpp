//---------------------------------------------------------------------------
/// \file   client/opengl_vertex.hpp
/// \brief  Composition and binding of OpenGL vertex attributes
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
// Copyright 2013-2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

/** Example code:
 * @code

// Very simple vertex type, only has a position.
typedef vertex_1<vtx_xyz<float> > simple_vtx;

// Vertex type with a 16-bit position, texture coordinates, and
// an RGB color.
typedef vertex_4<vtx_xyz<int16_t>,
                 vtx_uv<float>,
                 vtx_rgb<uint8_t>,
                 vtx_padding<1>>     vtx;

// Build a model...
std::vector<vtx> vertices;

// First vertex is at position (1, 2, 3), UV coordinates,  white color...
vertices.emplace_back({ {1, 2, 3}, { 0.4f, 0.3f }, { 255, 255, 255 } });

// Done, make a VBO.
auto buffer = gl::make_vbo(vertices);

// Draw the model.
buffer.bind();
bind_attributes<vtx>();
buffer.draw_triangles();

 * @endcode
 */

#include <array>
#include <initializer_list>
#include "opengl.hpp"

namespace hexa
{

// Map C++ types to OpenGL enumerations

template <class t>
struct gl_type
{
    GLenum operator()() { return 0; }
};

template <>
struct gl_type<uint8_t>
{
    GLenum operator()() { return GL_UNSIGNED_BYTE; }
};

template <>
struct gl_type<int8_t>
{
    GLenum operator()() { return GL_BYTE; }
};

template <>
struct gl_type<uint16_t>
{
    GLenum operator()() { return GL_UNSIGNED_SHORT; }
};

template <>
struct gl_type<int16_t>
{
    GLenum operator()() { return GL_SHORT; }
};

template <>
struct gl_type<uint32_t>
{
    GLenum operator()() { return GL_UNSIGNED_INT; }
};

template <>
struct gl_type<int32_t>
{
    GLenum operator()() { return GL_INT; }
};

template <>
struct gl_type<float>
{
    GLenum operator()() { return GL_FLOAT; }
};

//---------------------------------------------------------------------------

// The following structs define the data that can be put in a vertex

#pragma pack(push, 1)

/** Texture coordinates */
template <class t = float>
struct vtx_uv : public vector2<t>
{
    vtx_uv() {}
    vtx_uv(const vector2<t>& init)
        : vector2<t>(init)
    {
    }
    vtx_uv(std::initializer_list<t> init)
        : vector2<t>(init)
    {
    }
    vtx_uv(const t* init)
        : vector2<t>(init)
    {
    }

    static void bind(size_t i, size_t o, size_t stride)
    {
        glCheck(glVertexAttribPointer(i, 2, gl_type<t>()(), GL_FALSE, stride,
                              (GLvoid*)o));
    }
    
    static bool is_padding() { return false; }
};

/** Vertex 3D coordinates */
template <class t = float>
struct vtx_xyz : public vector3<t>
{
    vtx_xyz() {}
    vtx_xyz(const vector3<t>& init)
        : vector3<t>(init)
    {
    }
    vtx_xyz(std::initializer_list<t> init)
        : vector3<t>(init)
    {
    }
    vtx_xyz(const t* init)
        : vector3<t>(init)
    {
    }

    static void bind(size_t i, size_t o, size_t stride)
    {
        glCheck(glVertexAttribPointer(i, 3, gl_type<t>()(), GL_FALSE, stride,
                              (GLvoid*)o));
    }
    
    static bool is_padding() { return false; }    
};

/** Vertex normal */
template <class t = float>
struct vtx_normal : public vector3<t>
{
    vtx_normal() {}
    vtx_normal(const vector3<t>& init)
        : vector3<t>(init)
    {
    }
    vtx_normal(std::initializer_list<t> init)
        : vector3<t>(init)
    {
    }
    vtx_normal(const t* init)
        : vector3<t>(init)
    {
    }

    static void bind(size_t i, size_t o, size_t stride)
    {
        glCheck(glVertexAttribPointer(i, 3, gl_type<t>()(), GL_FALSE, stride,
                              (GLvoid*)o));
    }
    
    static bool is_padding() { return false; }
};

/** Vertex color */
template <class t = uint8_t>
struct vtx_rgb : public vector3<t>
{
    vtx_rgb() {}
    vtx_rgb(const vector3<t>& init)
        : vector3<t>(init)
    {
    }
    vtx_rgb(const t* init)
        : vector3<t>(init)
    {
    }

    static void bind(size_t i, size_t o, size_t stride)
    {
        glCheck(glVertexAttribPointer(i, 3, gl_type<t>()(), GL_TRUE, stride,
                              (GLvoid*)o));
    }
    
    static bool is_padding() { return false; }
};

/** Scalar value */
template <class t = float>
struct vtx_scalar
{
    vtx_scalar() {}
    vtx_scalar(t init)
        : value(init)
    {
    }

    static void bind(size_t i, size_t o, size_t stride)
    {
        glCheck(glVertexAttribIPointer(i, 1, gl_type<t>()(), stride, (GLvoid*)o));
    }
    
    static bool is_padding() { return false; }

    t value;
};

/** Array of scalars */
template <class t, size_t count>
struct vtx_array : public std::array<t, count>
{
    vtx_array() {}
    vtx_array(const std::array<t, count>& init)
        : std::array<t, count>(init)
    {
    }
    vtx_array(const t* init)
    {
        for (size_t i(0); i < count; ++i)
            (*this)[i] = init[i];
    }

    vtx_array(std::initializer_list<t> init)
    {
        assert(init.size() == count);
        std::copy(init.begin(), init.end(), this->begin());
    }

    static void bind(size_t i, size_t o, size_t stride)
    {
        glCheck(glVertexAttribIPointer(i, count, gl_type<t>()(), stride, (GLvoid*)o));
    }
    
    static bool is_padding() { return false; }    
};

/** Array of normalized scalars.
 *  Normalized scalars are mapped to a 0..1 range.  So if \a t is an
 *  uint8_t, a value of 128 would be interpreted as 0.5. */
template <class t, size_t count>
struct vtx_normalized_array : public std::array<t, count>
{
    vtx_normalized_array() {}
    vtx_normalized_array(const std::array<t, count>& init)
        : std::array<t, count>(init)
    {
    }
    vtx_normalized_array(const t* init)
    {
        for (size_t i(0); i < count; ++i)
            (*this)[i] = init[i];
    }

    vtx_normalized_array(std::initializer_list<t> init)
    {
        assert(init.size() == count);
        std::copy(init.begin(), init.end(), this->begin());
    }

    static void bind(size_t i, size_t o, size_t stride)
    {
        glCheck(glVertexAttribPointer(i, count, gl_type<t>()(), true, stride,
                                     (GLvoid*)o));
    }
    
    static bool is_padding() { return false; }
};

/** Vertex data padding.
 *  This can be used to make sure a vertex always ends at a 4 byte
 *  boundary, as required by most OpenGL drivers. */
template <size_t count>
struct vtx_padding : public std::array<char, count>
{
    static void bind(size_t i, size_t o, size_t stride) {}
    
    static bool is_padding() { return true; }    
};

//---------------------------------------------------------------------------

/** OpenGL vertex type.
 *  A vertex can hold a position, color, texture coordinate, or general
 *  scalar values, in any order and combination. */
template <class elem1>
class vertex_1
{
public:
    enum { element_count = 1 };

    typedef elem1 value_type_1;
    typedef elem1 value_type_2;
    typedef elem1 value_type_3;
    typedef elem1 value_type_4;
    typedef elem1 value_type_5;

public:
    vertex_1() {}

    vertex_1(elem1 i1)
        : e1(i1)
    {
    }

public:
    elem1 e1;
};

template <class elem1, class elem2>
class vertex_2
{
public:
    enum { element_count = 2 };

    typedef elem1 value_type_1;
    typedef elem2 value_type_2;
    typedef elem2 value_type_3;
    typedef elem2 value_type_4;
    typedef elem2 value_type_5;

public:
    vertex_2() {}

    vertex_2(elem1 i1, elem2 i2 = elem2())
        : e1(i1)
        , e2(i2)
    {
    }

public:
    elem1 e1;
    elem2 e2;
};

template <class elem1, class elem2, class elem3>
class vertex_3
{
public:
    enum { element_count = 3 };

    typedef elem1 value_type_1;
    typedef elem2 value_type_2;
    typedef elem3 value_type_3;
    typedef elem3 value_type_4;
    typedef elem3 value_type_5;

public:
    vertex_3() {}

    vertex_3(elem1 i1, elem2 i2, elem3 i3 = elem3())
        : e1(i1)
        , e2(i2)
        , e3(i3)
    {
    }

public:
    elem1 e1;
    elem2 e2;
    elem3 e3;
};

template <class elem1, class elem2, class elem3, class elem4>
class vertex_4
{
public:
    enum { element_count = 4 };

    typedef elem1 value_type_1;
    typedef elem2 value_type_2;
    typedef elem3 value_type_3;
    typedef elem4 value_type_4;
    typedef elem4 value_type_5;

public:
    vertex_4() {}

    vertex_4(elem1 i1, elem2 i2, elem3 i3, elem4 i4 = elem4())
        : e1(i1)
        , e2(i2)
        , e3(i3)
        , e4(i4)
    {
    }

public:
    elem1 e1;
    elem2 e2;
    elem3 e3;
    elem4 e4;
};

template <class elem1, class elem2, class elem3, class elem4, class elem5>
class vertex_5
{
public:
    enum { element_count = 5 };

    typedef elem1 value_type_1;
    typedef elem2 value_type_2;
    typedef elem3 value_type_3;
    typedef elem4 value_type_4;
    typedef elem5 value_type_5;

public:
    vertex_5() {}

    vertex_5(elem1 i1, elem2 i2, elem3 i3, elem4 i4, elem5 i5 = elem5())
        : e1(i1)
        , e2(i2)
        , e3(i3)
        , e4(i4)
        , e5(i5)
    {
    }

public:
    elem1 e1;
    elem2 e2;
    elem3 e3;
    elem4 e4;
    elem5 e5;
};

#pragma pack(pop)

//---------------------------------------------------------------------------

template <typename Vertex>
void bind_attributes()
{
    size_t offset(0), size(sizeof(Vertex));
    Vertex::value_type_1::bind(0, offset, size);

    if (Vertex::element_count > 1) {
        offset += sizeof(typename Vertex::value_type_1);
        Vertex::value_type_2::bind(1, offset, size);
    }

    if (Vertex::element_count > 2) {
        offset += sizeof(typename Vertex::value_type_2);
        Vertex::value_type_3::bind(2, offset, size);
    }

    if (Vertex::element_count > 3) {
        offset += sizeof(typename Vertex::value_type_3);
        Vertex::value_type_4::bind(3, offset, size);
    }

    if (Vertex::element_count > 4) {
        offset += sizeof(typename Vertex::value_type_4);
        Vertex::value_type_5::bind(4, offset, size);
    }
}

template <typename Vertex>
void bind_attributes(const std::vector<int>& attrs)
{
    size_t offset(0), size(sizeof(Vertex));
    Vertex::value_type_1::bind(attrs[0], offset, size);

    if (Vertex::element_count > 1) {
        offset += sizeof(typename Vertex::value_type_1);
        Vertex::value_type_2::bind(attrs[1], offset, size);
    }

    if (Vertex::element_count > 2) {
        offset += sizeof(typename Vertex::value_type_2);
        Vertex::value_type_3::bind(attrs[2], offset, size);
    }

    if (Vertex::element_count > 3) {
        offset += sizeof(typename Vertex::value_type_3);
        Vertex::value_type_4::bind(attrs[3], offset, size);
    }

    if (Vertex::element_count > 4) {
        offset += sizeof(typename Vertex::value_type_4);
        Vertex::value_type_5::bind(attrs[4], offset, size);
    }
}

template <typename Vertex>
void enable_attrib_array()
{
    if (!Vertex::value_type_1::is_padding())
        glCheck(glEnableVertexAttribArray(0));

    if (Vertex::element_count > 1 && !Vertex::value_type_2::is_padding())
        glCheck(glEnableVertexAttribArray(1));

    if (Vertex::element_count > 2 && !Vertex::value_type_3::is_padding())
        glCheck(glEnableVertexAttribArray(2));

    if (Vertex::element_count > 3 && !Vertex::value_type_4::is_padding())
        glCheck(glEnableVertexAttribArray(3));
    
    if (Vertex::element_count > 4 && !Vertex::value_type_5::is_padding())
        glCheck(glEnableVertexAttribArray(4));
}

template <typename Vertex>
void disable_attrib_array()
{
    for (int i = 0; i < Vertex::element_count; ++i)
        glCheck(glDisableVertexAttribArray(i));
}

} // namespace hexa
