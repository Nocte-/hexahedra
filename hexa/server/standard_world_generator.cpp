//---------------------------------------------------------------------------
// server/standard_world_generator.cpp
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

#include "standard_world_generator.hpp"

#include <stdexcept>
#include <mutex>
#include <unordered_map>
#include <noisepp/NoisePerlin.h>
#include <noisepp/NoisePipeline.h>

#include <hexa/block_types.hpp>
#include <hexa/trace.hpp>

#include "world.hpp"

using namespace boost::property_tree;

namespace hexa {

struct standard_world_generator::impl
{
    world& map;

    uint16_t    height_idx;
    uint16_t    rough_idx;
    uint16_t    temp_idx;

    noisepp::PerlinModule       noise;
    noisepp::Pipeline3D         pipeline;
    noisepp::ElementID          noise_id;
    noisepp::Cache*             noise_cache;
    noisepp::PipelineElement3D* perlin;

    double granularity;
    double rough_size;

    uint16_t grass_id;
    uint16_t dirt_id;
    uint16_t rock_id;
    uint16_t sand_id;
    uint16_t snow_id;

    impl(world& w)
        : map         (w)
        , noise_cache (nullptr)
    {
        int i (w.find_area_generator("heightmap"));
        if (i < 0)
            throw std::runtime_error("standard_terrain_generator requires a height map");

        height_idx = static_cast<uint16_t>(i);
        rough_idx  = static_cast<uint16_t>(w.find_area_generator("rough_terrain"));
        temp_idx   = static_cast<uint16_t>(w.find_area_generator("temperature"));

        noise.setOctaveCount(3);
        noise_id = noise.addToPipe(pipeline);
        perlin = pipeline.getElement(noise_id);
        noise_cache = pipeline.createCache();

        granularity = 40.;
        rough_size = 30.;

        grass_id = find_material("grass");
        dirt_id  = find_material("dirt");
        rock_id  = find_material("stone");
        sand_id  = find_material("sand");
        snow_id  = find_material("snow");
    }

    ~impl()
    {
        pipeline.freeCache(noise_cache);
    }

    void generate(chunk_coordinates pos, chunk& dest)
    {
        trace((boost::format("Generate new chunk at %1%") % world_vector(pos - world_chunk_center)).str());

        if (   pos.x >= chunk_world_limit.x
            || pos.y >= chunk_world_limit.y
            || pos.z >= chunk_world_limit.z)
        {
            throw std::logic_error("not a valid chunk position");
        }

        world_coordinates offset (pos * chunk_size);
        area_ptr hm      (map.get_area_data(pos, height_idx));
        area_ptr tempmap (map.get_area_data(pos, temp_idx));
        area_ptr rm      (nullptr);

        if (rough_idx < 999)
            rm = map.get_area_data(pos, rough_idx);

        if (hm == nullptr)
            throw std::runtime_error("no height map was generated");

        boost::lock_guard<boost::mutex> lock (dest.lock);

        for (int x (0); x < chunk_size; ++x)
        {
            for (int y (0); y < chunk_size; ++y)
            {
                // Relative height: -32K to +32K around sea level
                int16_t&  rel_h ((*hm)(x, y));
                // Roughness factor
                int16_t   r     (rm ? (*rm)(x, y) : 0);
                // Multiplication factor based on terrain roughness
                double    mul_b (clamp((r - 5000.) / 10000., 0.0, 2.0));

                // Absolute height
                uint32_t h (world_center.z + rel_h);

                for (int z (0); z < chunk_size; ++z)
                {
                    uint32_t rh (offset.z + z);

                    if (r > 0 && mul_b > 0)
                    {
                        double mul (mul_b);

                        // No rough terrain near the coast
                        if (rel_h < 50)
                            mul *= std::max((rel_h - 20.) / 30., 0.0);

                        vector3<double> pn (offset + world_coordinates(x,y,z));
                        pn /= granularity;
                        double p (perlin->getValue(pn.x, pn.y, pn.z, noise_cache) * rough_size);
                        rh += mul * p;
                    }

                    if (rh <= h)
                        dest(x,y,z) = rock_id;
                }
            }
        }
    }

    chunk_height estimate_height (map_coordinates xy) const
    {
        area_ptr hm (map.get_area_data(xy, height_idx));
        if (hm == nullptr)
            return undefined_height;

        return (water_level + rough_size + *std::max_element(hm->begin(), hm->end())) / chunk_size;
    }
};

standard_world_generator::standard_world_generator (world& w,
                                                    const ptree& conf)
    : terrain_generator_i (w, conf)
    , pimpl_ (new impl (w))
{ }

standard_world_generator::~standard_world_generator()
{ }

void standard_world_generator::generate(chunk_coordinates pos, chunk& dest)
{
    pimpl_->generate(pos, dest);
}

chunk_height
standard_world_generator::estimate_height (map_coordinates xy) const
{
    return pimpl_->estimate_height(xy);
}

} // namespace hexa

