//---------------------------------------------------------------------------
// client/model_opengl.cpp
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

#include "model_opengl.hpp"

#include <boost/range/algorithm.hpp>
#include "sfml_resource_manager.hpp"

using namespace boost::range;

namespace hexa {
namespace gl {

model::model(const hexa::model& init)
    : vertices (gl::make_vbo(init.vertex_buffer))
    , triangles (make_edge_buffer(init.edge_buffer))
{
    boost::range::copy(init.meshes, std::back_inserter(meshes));
}

model::mesh::mesh(const hexa::model::mesh& m)
    : first_triangle (m.first_triangle)
    , nr_of_triangles (m.nr_of_triangles)
    , tex (textures(m.material))
{ }

}} // namespace hexa::gl
