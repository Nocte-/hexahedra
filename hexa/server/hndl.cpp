//---------------------------------------------------------------------------
// server/hndl.cpp
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

#include "hndl.hpp"

#include <hexanoise/generator_opencl.hpp>
#include <hexanoise/generator_slowinterpreter.hpp>
#include <hexanoise/simple_global_variables.hpp>

#include <hexa/compiler_fix.hpp>
#include <hexa/crypto.hpp>
#include <hexa/log.hpp>
#include "opencl.hpp"

namespace hexa
{
namespace
{

noise::simple_global_variables glob_vars;
std::unique_ptr<noise::generator_context> gen_ctx;

} // anonymous namespace

std::unique_ptr<noise::generator_i> compile_hndl(const std::string& script)
{
    return compile_hndl(crypto::sha256(script), script);
}

std::unique_ptr<noise::generator_i> compile_hndl(const std::string& name,
                                                 const std::string& script)
{
    if (gen_ctx == nullptr)
        gen_ctx = std::make_unique<noise::generator_context>(glob_vars);

    auto& n = gen_ctx->set_script(name, script);

    /*
    if (have_opencl()) {
        try {
            return std::make_unique<noise::generator_opencl>(
                *gen_ctx, opencl_context(), opencl_device(), n);
        } catch (std::exception& e) {
            log_msg("Cannot set up OpenCL : %1%", e.what());
        }
    }
    */

    return std::make_unique<noise::generator_slowinterpreter>(*gen_ctx, n);
}

void set_global_variable(const std::string& name, double val)
{
    glob_vars[name] = val;
}

} // namespace hexa
