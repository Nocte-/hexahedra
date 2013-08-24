//---------------------------------------------------------------------------
// server/biome_generator.cpp
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
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "biome_generator.hpp"

#include <stdexcept>
#include <unordered_map>
#include <noisepp/NoisePerlin.h>
#include <noisepp/NoisePipeline.h>

using namespace boost::property_tree;

namespace hexa {

class biome_generator::impl
{
    noisepp::PerlinModule       noise;
    noisepp::Pipeline2D         pipeline;
    noisepp::ElementID          noise_id;
    noisepp::Cache*             noise_cache;
    noisepp::PipelineElement2D* perlin;

    double  size;
    double  offset;
    double  mul;

public:
    impl(int octaves, area_data::value_type low, area_data::value_type high,
         double scale)
        : noise_cache(nullptr)
    {
        // The output range of the Perlin noise generator is dependent on the
        // number of octaves used.  A single generator outputs -1..1, but
        // every additional octave extends that range by half.  We adjust
        // the low and high values accordingly.
        double range (2.0 - std::pow<double>(0.5, octaves - 1));
        low /= range;
        high /= range;

        noise.setOctaveCount(octaves);
        noise_id = noise.addToPipe(pipeline);
        perlin = pipeline.getElement(noise_id);
        noise_cache = pipeline.createCache();

        size = scale;
        offset = low;
        mul    = (high - low) * 0.5;
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
                double px ((o.x + x * 4) / size),
                       py ((o.y + y * 4) / size);

                double pn (perlin->getValue(px, py, noise_cache));
                temp[x+y*5] = (pn + 1.0) * mul + offset;
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


biome_generator::biome_generator (world& w, const ptree& conf)
    : area_generator_i (w, conf)
    , pimpl_ (new impl(conf.get<int>("octaves", 2),
                       conf.get<int>("min", -32000),
                       conf.get<int>("max", 32000),
                       conf.get<double>("scale", 1000.0)))

{ }

biome_generator::~biome_generator()
{ }

area_data& biome_generator::generate (map_coordinates xy, area_data& dest)
{
    pimpl_->generate(xy, dest);
    return dest;
}

} // namespace hexa

