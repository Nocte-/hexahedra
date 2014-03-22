//---------------------------------------------------------------------------
/// \file   server/opencl.hpp
/// \brief  OpenCL stuff
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
// Copyright (C) 2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#pragma once

namespace cl {
    class Context;
    class Device;
}

namespace hexa {

void init_opencl();

bool have_opencl();

cl::Context& opencl_context();

cl::Device& opencl_device();

} // namespace hexa

