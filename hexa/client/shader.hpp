//---------------------------------------------------------------------------
/// \file   client/shader.hpp
/// \brief  GLSL shader
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

#include <array>
#include <stdexcept>
#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include <hexa/matrix.hpp>

namespace hexa {

class color;

class shader_error : public std::runtime_error
{
public:
    shader_error(const std::string& what) : std::runtime_error(what) {}
};

class shader
{
public:
    typedef enum { vertex, fragment } type;

    shader();
    shader(shader&& move);
    ~shader();

    void load(type t, const std::string& code);
    GLuint id() const { return id_; }
    std::string info_log() const;

private:
    GLuint id_;
};

class shader_program;

class uniform_variable
{
public:
    uniform_variable();

    bool bind(const shader_program& prog, const std::string& name);

    bool is_bound() const { return bound_; }

    void operator= (int val);
    void operator= (const std::vector<int>& ints);
    void operator= (const std::array<int, 2>& ints);
    void operator= (const std::array<int, 3>& ints);
    void operator= (const std::array<int, 4>& ints);

    void operator= (float val);
    void operator= (const std::vector<float>& ints);
    void operator= (const std::array<float, 2>& ints);
    void operator= (const std::array<float, 3>& ints);
    void operator= (const std::array<float, 4>& ints);
    void operator= (const color& val);

    void operator= (const matrix4<float>& mtx);

private:
    GLint id_;
    bool  bound_;
};

class shader_program
{
public:
    shader_program();
    shader_program(shader_program&& move);
    ~shader_program();

    void load(const boost::filesystem::path& base);

    GLuint id() const { return id_; }
    GLuint vs_id() const { return vertex_.id(); }
    GLuint fs_id() const { return fragment_.id(); }

    void bind_attribute(unsigned int index, const std::string& name);
    unsigned int get_attribute(const std::string& name) const;

    bool link() const;
    std::string info_log() const;

    void use() const;
    void stop_using() const;

private:
    GLuint id_;
    shader vertex_;
    shader fragment_;
};

} // namespace hexa

