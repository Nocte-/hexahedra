//---------------------------------------------------------------------------
/// \file   client/model.hpp
/// \brief  A 3-D model with skeletal animation.
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

#include <array>
#include <string>
#include <vector>
#include <boost/utility.hpp>

#include <hexa/matrix.hpp>
#include <hexa/quaternion.hpp>

#include <hexa/client/opengl_vertex.hpp>
#include <hexa/client/texture.hpp>
#include <hexa/client/vbo.hpp>

namespace hexa {

class model
{
public:
    model() {}
	/*
    model(const model&) = delete;
    model& operator= (const model&) = delete;

    model(model&&) = default;
    model& operator= (model&&) = default;
	*/
    //typedef ogl::vertex<vtx_xyz<>, vtx_uv<>, vtx_normal<>> vertex;
    typedef vertex_5<vtx_xyz<>, vtx_uv<>, vtx_normal<>,
                     vtx_normalized_array<uint8_t, 4>, // blend weights
                     vtx_array<uint8_t, 4>             // blend indices
                     > vertex;

    typedef vertex_5<vtx_xyz<>, vtx_rgb<>, vtx_normal<>,
                     vtx_normalized_array<uint8_t, 4>, // blend weights
                     vtx_array<uint8_t, 4>             // blend indices
                     > vertex_col;

    struct mesh
    {
        mesh(uint16_t ft, uint16_t nt, const std::string& mat)
            : first_triangle(ft)
            , nr_of_triangles(nt)
            , material(mat)
        { }

        uint16_t                first_triangle;
        uint16_t                nr_of_triangles;

        std::string             material;
        texture                 tex;
        texture                 specular;
    };

    struct bone
    {
        bone(vector3<float> t, quaternion<float> r,
             vector3<float> s, uint16_t p, std::string n)
            : translation(t), rotation(r)
            , scale(s), parent(p)
            , name(n)
        { }

        vector3<float>          translation;
        quaternion<float>       rotation;
        vector3<float>          scale;

        uint16_t                parent;
        std::string             name;
    };

    typedef std::array<uint16_t, 3> triangle;

    std::vector<vertex>     vertex_buffer;
    std::vector<vertex_col> vertex_buffer_col;
    std::vector<triangle>   edge_buffer;
    std::vector<mesh>       meshes;
    std::vector<bone>       bones;

    std::string             name;
};

class animation : boost::noncopyable
{
public:
    std::string         name;
    float               framerate;
    uint32_t            flags;

    struct pose
    {
        uint16_t                        parent;
        uint16_t                        mask;
        std::vector<matrix3x4<float>>   data;
    };

    std::vector<pose>   frames;
};

} // namespace hexa

