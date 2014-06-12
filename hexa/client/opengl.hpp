//---------------------------------------------------------------------------
/// \file   client/opengl.hpp
/// \brief  OpenGL utilities
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

#include <vector>
#include <initializer_list>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include "../aabb.hpp"
#include "../basic_types.hpp"
#include "../color.hpp"
#include "../vector2.hpp"
#include "../vector3.hpp"

// glCheck() only checks for errors in debug.
//#ifndef NDEBUG
#define glCheck(x) ((x), hexa::gl::check_error(__FILE__, __LINE__))
//#else
//#  define glCheck(x) (x)
//#endif

namespace hexa
{
namespace gl
{

void check_error(const char* file, unsigned int line);

inline void color(const hexa::color& c)
{
    glColor3f(c.r(), c.g(), c.b());
}

inline void color(const hexa::color_alpha& c)
{
    glColor4f(c.r(), c.g(), c.b(), c.a());
}

inline void vertex(const hexa::vector3<int>& p)
{
    glVertex3i(p.x, p.y, p.z);
}

inline void vertex(const hexa::vector3<float>& p)
{
    glVertex3f(p.x, p.y, p.z);
}

inline void tex_coord(const hexa::vector2<float>& p)
{
    glTexCoord2f(p.x, p.y);
}

inline void translate(const hexa::vector3<float>& p)
{
    glTranslatef(p.x, p.y, p.z);
}

inline void enable(int op)
{
    glCheck(glEnable(op));
}

inline void enable(std::initializer_list<int> ops)
{
    for (auto o : ops)
        glCheck(glEnable(o));
}

inline void disable(int op)
{
    glCheck(glDisable(op));
}

inline void disable(std::initializer_list<int> ops)
{
    for (auto o : ops)
        glCheck(glDisable(o));
}

class client_states
{
    std::vector<int> options_;

public:
    client_states(std::vector<int> ops)
        : options_(ops)
    {
        for (auto o : options_)
            glCheck(glEnableClientState(o));
    }

    client_states(std::initializer_list<int> ops)
        : options_(ops)
    {
        for (auto o : options_)
            glCheck(glEnableClientState(o));
    }

    ~client_states()
    {
        for (auto o : options_)
            glCheck(glDisableClientState(o));
    }
};

class guard_matrix
{
public:
    guard_matrix() { glPushMatrix(); }
    ~guard_matrix() { glPopMatrix(); }
};

void cube_face(float size, direction_type d, float grow = 0.001f);

void box(const aabb<vector>& box);
}
} // namespace hexa::opengl
