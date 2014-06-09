//---------------------------------------------------------------------------
/// \file   client/model_opengl.hpp
/// \brief  A model that can be rendered in OpenGL
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

#include "model.hpp"
#include "edge_buffer.hpp"
#include "vbo.hpp"
#include "resource_manager.hpp"

namespace hexa
{
namespace gl
{

class model
{
public:
    typedef hexa::model::vertex vertex;
    typedef hexa::model::vertex_col vertex_col;
    typedef hexa::model::triangle triangle;

public:
    model(const hexa::model& init);
    /*
model(const model&) = delete;
model& operator= (const model&) = delete;

model(model&&) = default;
model& operator= (model&&) = default;
    */

    struct mesh
    {
        mesh(const hexa::model::mesh& m);

        uint16_t first_triangle;
        uint16_t nr_of_triangles;
        resource_manager<texture>::resource tex;
    };

    std::vector<mesh> meshes;
    vbo vertices;
    edge_buffer<uint16_t> triangles;
};
}
} // namespace hexa::gl
