//---------------------------------------------------------------------------
// client/shader.cpp
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

#include "shader.hpp"

#include <hexa/algorithm.hpp>
#include <hexa/color.hpp>
#include <hexa/log.hpp>

#include "opengl.hpp"

namespace fs = boost::filesystem;

namespace hexa
{

shader::shader()
    : id_{0}
{
}

void shader::load(type t, const std::string& code)
{
    id_ = glCreateShader(t == vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
    if (id_ == 0)
        throw shader_error("glCreateShader failed");

    const char* ptr = code.c_str();
    glCheck(glShaderSource(id_, 1, &ptr, nullptr));
    glCheck(glCompileShader(id_));
}

shader::shader(shader&& move)
    : id_{move.id_}
{
    move.id_ = 0;
}

shader::~shader()
{
    if (id_ != 0)
        glCheck(glDeleteShader(id_));
}

std::string shader::info_log() const
{
    char temp[4096];
    int len{0};
    glCheck(glGetShaderInfoLog(id_, 4096, &len, temp));
    return std::string(temp, len);
}

/////////////////////////////////////////////////////////////////////////////

uniform_variable::uniform_variable()
    : id_{0}
    , bound_{false}
{
}

bool uniform_variable::bind(const shader_program& prog,
                            const std::string& name)
{
    bound_ = true;
    id_ = glGetUniformLocation(prog.id(), name.c_str());
    if (id_ == -1) {
        id_ = 0;
        bound_ = false;
        log_msg("Warning, could not bind uniform '%1%'", name);
    }
    return bound_;
}

void uniform_variable::operator=(int val)
{
    if (bound_)
        glCheck(glUniform1i(id_, val));
}

void uniform_variable::operator=(const std::vector<int>& ints)
{
    if (!bound_)
        return;

    switch (ints.size()) {
    case 0:
        return;
    case 1:
        glCheck(glUniform1i(id_, ints[0]));
        break;
    case 2:
        glCheck(glUniform2i(id_, ints[0], ints[1]));
        break;
    case 3:
        glCheck(glUniform3i(id_, ints[0], ints[1], ints[2]));
        break;
    case 4:
        glCheck(glUniform4i(id_, ints[0], ints[1], ints[2], ints[3]));
        break;
    }
}

void uniform_variable::operator=(const std::array<int, 2>& ints)
{
    if (bound_)
        glCheck(glUniform2i(id_, ints[0], ints[1]));
}

void uniform_variable::operator=(const std::array<int, 3>& ints)
{
    if (bound_)
        glCheck(glUniform3i(id_, ints[0], ints[1], ints[2]));
}

void uniform_variable::operator=(const std::array<int, 4>& ints)
{
    if (bound_)
        glCheck(glUniform4i(id_, ints[0], ints[1], ints[2], ints[3]));
}

void uniform_variable::operator=(float val)
{
    if (bound_)
        glCheck(glUniform1f(id_, val));
}

void uniform_variable::operator=(const std::vector<float>& flts)
{
    if (!bound_)
        return;

    switch (flts.size()) {
    case 0:
        return;
    case 1:
        glCheck(glUniform1f(id_, flts[0]));
        break;
    case 2:
        glCheck(glUniform2f(id_, flts[0], flts[1]));
        break;
    case 3:
        glCheck(glUniform3f(id_, flts[0], flts[1], flts[2]));
        break;
    case 4:
        glCheck(glUniform4f(id_, flts[0], flts[1], flts[2], flts[3]));
        break;
    }
}

void uniform_variable::operator=(const std::array<float, 2>& flts)
{
    if (bound_)
        glCheck(glUniform2f(id_, flts[0], flts[1]));
}

void uniform_variable::operator=(const std::array<float, 3>& flts)
{
    if (bound_)
        glCheck(glUniform3f(id_, flts[0], flts[1], flts[2]));
}

void uniform_variable::operator=(const std::array<float, 4>& flts)
{
    if (bound_)
        glCheck(glUniform4f(id_, flts[0], flts[1], flts[2], flts[3]));
}

void uniform_variable::operator=(const color& c)
{
    if (bound_)
        glCheck(glUniform3f(id_, c[0], c[1], c[2]));
}

void uniform_variable::operator=(const matrix4<float>& m)
{
    if (bound_)
        glCheck(glUniformMatrix4fv(id_, 1, GL_FALSE, m.as_ptr()));
}

/////////////////////////////////////////////////////////////////////////////

shader_program::shader_program()
    : id_(0)
{
}

shader_program::shader_program(shader_program&& move)
    : id_(move.id_)
{
    move.id_ = 0;
}

shader_program::~shader_program()
{
    if (id_ != 0) {
        glCheck(glDetachShader(id_, fs_id()));
        glCheck(glDetachShader(id_, vs_id()));
        glCheck(glDeleteProgram(id_));
    }
}

void shader_program::load(const fs::path& base)
{
    if (id_ != 0)
        glCheck(glDeleteProgram(id_));

    id_ = glCreateProgram();
    if (id_ == 0)
        throw std::runtime_error("cannot create GLSL shader");

    fs::path temp(base);
    vertex_.load(shader::vertex, file_contents(temp.replace_extension(".vs")));

    fragment_.load(shader::fragment,
                   file_contents(temp.replace_extension(".fs")));

    glCheck(glAttachShader(id_, vs_id()));
    glCheck(glAttachShader(id_, fs_id()));
}

std::string shader_program::info_log() const
{
    char temp[4096];
    int len(0);
    glCheck(glGetProgramInfoLog(id_, 4096, &len, temp));
    return vertex_.info_log() + "\n" + fragment_.info_log() + "\n"
           + std::string(temp, len);
}

void shader_program::bind_attribute(unsigned int index,
                                    const std::string& name)
{
    assert(id_ != 0);
    glCheck(glBindAttribLocation(id_, index, name.c_str()));
}

unsigned int shader_program::get_attribute(const std::string& name) const
{
    assert(id_ != 0);
    return glGetAttribLocation(id_, name.c_str());
}

bool shader_program::link() const
{
    assert(id_ != 0);
    glCheck(glLinkProgram(id_));

    GLint flag;
    glCheck(glGetProgramiv(id_, GL_LINK_STATUS, &flag));
    return flag != 0;
}

void shader_program::use() const
{
    assert(id_ != 0);
    glCheck(glUseProgram(id_));
}

void shader_program::stop_using() const
{
    glCheck(glUseProgram(0));
}

} // namespace hexa
