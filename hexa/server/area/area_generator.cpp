//---------------------------------------------------------------------------
// server/area/area_generator.hpp
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
// Copyright 2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "area_generator.hpp"

#include <hexanoise/generator_opencl.hpp>
#include <hexanoise/generator_slowinterpreter.hpp>
#include <hexa/algorithm.hpp>
#include "../opencl.hpp"

namespace hexa {

area_generator::area_generator (world& w,
                                const boost::property_tree::ptree& conf,
                                const noise::generator_context& ctx)
    : area_generator_i(w, conf)
    , ctx_(ctx)
    , script_(ctx_.get_script(conf.get<std::string>("name")))
{
    if (have_opencl())
    {
        try
        {
            gen_ = std::make_unique<noise::generator_opencl>(ctx, opencl_context(), opencl_device(), script_);
        }
        catch (std::exception& e)
        {
            std::cout << "Cannot set up OpenCL : " << e.what() << std::endl;
        }
    }

    if (gen_ == nullptr)
        gen_ = std::make_unique<noise::generator_slowinterpreter>(ctx, script_);

    std::string t (conf.get<std::string>("type", "regular"));

    if (t == "normalized")
        type_ = type::normalized;
    else
        type_ = type::regular;
}

area_data
area_generator::generate (map_coordinates pos)
{
    area_data result;
    auto tmp (gen_->run(glm::dvec2((int)pos.x - (int)world_chunk_center.x, (int)pos.y - (int)world_chunk_center.y) * (double)chunk_size,
                        glm::dvec2(1, 1),
                        glm::ivec2(chunk_size, chunk_size) ));

    switch (type_)
    {
    case type::normalized:
        std::transform(tmp.begin(), tmp.end(), result.begin(),
                       [](double i){ return std::lround(clamp(i, -1.0, 0.0) * 32767); });
        break;

    case type::regular:
        std::transform(tmp.begin(), tmp.end(), result.begin(),
                       [](double i){ return std::lround(clamp(i, -32767.0,  32767.0)); });
        break;

    default:
        assert(false);
    }
    return result;
}

} // namespace hexa

