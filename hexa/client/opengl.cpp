//---------------------------------------------------------------------------
// hexa/client/opengl.cpp
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

#include "opengl.hpp"

#include <hexa/log.hpp>

namespace hexa {
namespace gl {

void check_error(const char* file, unsigned int line)
{
    auto errorCode (glGetError());

    if (errorCode == GL_NO_ERROR)
        return;

    std::string error ("unknown error");
    std::string description;

    switch (errorCode)
    {
        case GL_INVALID_ENUM:
            return; // too many false positives on virtualbox
            error = "GL_INVALID_ENUM";
            description = "an unacceptable value has been specified for an enumerated argument";
            break;

        case GL_INVALID_VALUE:
            error = "GL_INVALID_VALUE";
            description = "a numeric argument is out of range";
            break;

        case GL_INVALID_OPERATION:
            error = "GL_INVALID_OPERATION";
            description = "the specified operation is not allowed in the current state";
            break;

        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "GL_INVALID_FRAMEBUFFER_OPERATION";
            description = "the framebuffer object is not complete";
            break;

        case GL_OUT_OF_MEMORY:
            error = "GL_OUT_OF_MEMORY";
            description = "there is not enough memory left to execute the command";
            break;

        case GL_STACK_UNDERFLOW:
            error = "GL_STACK_UNDERFLOW";
            description = "an attempt has been made to perform an operation that would cause an internal stack to underflow";
            break;

        case GL_STACK_OVERFLOW:
            error = "GL_STACK_OVERFLOW";
            description = "an attempt has been made to perform an operation that would cause an internal stack to overflow";
            break;

        default:
            ;
    }

    log_msg("An OpenGL call failed in %1% line %2%:", file, line);
    log_msg("%1%, %2%: %3%", errorCode, error, description);
}

}} // namespace hexa::gl

