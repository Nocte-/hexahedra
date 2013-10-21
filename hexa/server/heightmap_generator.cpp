//---------------------------------------------------------------------------
// hexa/server/heightmap_generator.cpp
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
// Copyright 2012-2013, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "heightmap_generator.hpp"

#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <noisepp/NoiseCurve.h>
#include <noisepp/NoisePerlin.h>
#include <noisepp/NoisePipeline.h>

#include <hexa/algorithm.hpp>
#include <hexa/block_types.hpp>
#include <hexa/compiler_fix.hpp>

using namespace boost::property_tree;

namespace hexa {

struct heightmap_generator::impl
{
    noisepp::PerlinModule       noise;
    noisepp::CurveModule        curve;
    noisepp::Pipeline2D         pipeline;
    noisepp::ElementID          noise_id;
    noisepp::ElementID          curve_id;
    noisepp::Cache*             noise_cache;
    noisepp::PipelineElement2D* perlin;

    double  continent_size;
    double  continent_height;

    impl(const ptree& conf)
        : noise_cache (nullptr)
    {
        noise.setOctaveCount(conf.get<unsigned int>("octaves", 10));

        curve.addControlPoint(-1, -0.9);
        curve.addControlPoint(-0.1, -0.07);
        curve.addControlPoint(-0.01, -0.005);
        curve.addControlPoint(0, 0);
        curve.addControlPoint(0.01, 0.005);
        curve.addControlPoint(0.1, 0.07);
        curve.addControlPoint(1, 0.9);

        curve.setSourceModule(0, &noise);
        curve_id = curve.addToPipe(pipeline);

        perlin = pipeline.getElement(curve_id);
        noise_cache = pipeline.createCache();

        continent_size = conf.get<double>("scale", 16000.);
        continent_height = conf.get<double>("height", 1500);
    }

    ~impl()
    {
        pipeline.freeCache(noise_cache);
    }

    void generate(map_coordinates pos, area_data& dest)
    {
        assert (pos.x < chunk_world_limit.x);
        assert (pos.y < chunk_world_limit.y);

        const map_coordinates o (pos * chunk_size);

        // This temporary 5x5 array will hold the samples of Perlin noise.
        // Later, it will be extrapolated to 16x16.
        //
        std::array<double, 5*5> temp;
        for (uint32_t y (0) ; y < 5; ++y)
        {
            for (uint32_t x (0) ; x < 5; ++x)
            {
                double px ((o.x + x * 4) / continent_size),
                       py ((o.y + y * 4) / continent_size);

                double pv (perlin->getValue(px, py, noise_cache));
                auto height (pv * continent_height);
                temp[x+y*5] = height;
            }
        }

        // Extrapolate it and copy it to dest.
        //
        for (uint32_t y (0) ; y < 4; ++y)
        {
            for (uint32_t x (0) ; x < 4; ++x)
            {
                size_t ofs (x + y * 5);

                for (int sx (0); sx < 4; ++sx)
                {
                    for (int sy (0); sy < 4; ++sy)
                    {
                        dest(x*4 + sx, y*4 + sy) =
                            (
                              temp[ofs  ] * ((4-sx) * (4-sy))
                            + temp[ofs+1] * (   sx  * (4-sy))
                            + temp[ofs+5] * ((4-sx) *    sy )
                            + temp[ofs+6] * (   sx  *    sy )) * 0.25 * 0.25;
                    }
                }
            }
        }
    }
};

heightmap_generator::heightmap_generator (world& w, const ptree& conf)
    : area_generator_i (w, conf)
    , pimpl_(std::make_unique<impl>(conf))
{ }

heightmap_generator::~heightmap_generator()
{ }

area_data&
heightmap_generator::generate (map_coordinates pos, area_data& dest)
{
    pimpl_->generate(pos, dest);
    return dest;
}

} // namespace hexa

