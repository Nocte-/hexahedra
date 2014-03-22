//---------------------------------------------------------------------------
// server/opencl.cpp
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

#include "opencl.hpp"

#include <cassert>
#include <cstdint>

#ifdef HAVE_OPENCL

#include <clew/clew.h>
#include <CL/cl.hpp>

#ifdef WIN32
# define OPENCL_DLL_NAME "OpenCL.dll"
#elif defined (MACOSX)
# define OPENCL_DLL_NAME 0
#else
# define OPENCL_DLL_NAME "libOpenCL.so"
#endif


namespace hexa {

namespace {

bool initialized_ = false;
bool have_opencl_ = false;

cl::Context ctx_;
cl::Device  dev_;

}

void init_opencl()
{
    if (initialized_)
        return;

    initialized_ = true;

    if (clewInit(OPENCL_DLL_NAME) < 0)
        return;

    try
    {
        uint32_t platform_index (0);
        uint32_t device_index   (0);

        std::vector<cl::Platform> platform_list;
        cl::Platform::get(&platform_list);

        if (platform_index >= platform_list.size())
            return;

        std::vector<cl::Device> device_list;
        auto& pl (platform_list[platform_index]);
        pl.getDevices(CL_DEVICE_TYPE_ALL, &device_list);

        if (device_index >= device_list.size())
            return;

        cl_context_properties properties [] =
                    { CL_CONTEXT_PLATFORM, (cl_context_properties)(pl)(), 0 };

        ctx_ = cl::Context(CL_DEVICE_TYPE_ALL, properties);
        dev_ = device_list[device_index];

        have_opencl_ = true;
    }
    catch (...)
    {
        have_opencl_ = false;
    }
}

bool have_opencl()
{
    return initialized_ && have_opencl_;
}

cl::Context& opencl_context()
{
    assert(have_opencl());
    return ctx_;
}

cl::Device& opencl_device()
{
    assert(have_opencl());
    return dev_;
}

} // namespace hexa

#else

void init_opencl()  { }
bool have_opencl()  { return false; }
cl::Context& opencl_context() { throw 0; }
cl::Device& opencl_device() { throw 0; }

#endif
