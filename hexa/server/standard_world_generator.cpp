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
#include <boost/range/algorithm.hpp>
#include <noisepp/NoisePerlin.h>
#include <noisepp/NoisePipeline.h>

#include <hexa/block_types.hpp>
#include <hexa/trace.hpp>

#include "world.hpp"

using namespace boost::property_tree;
using namespace boost::range;

namespace hexa {

struct standard_world_generator::impl
{
    world& map;

    int    height_idx;
    int    rough_idx;
    int    surf_idx;

    noisepp::PerlinModule       noise;
    noisepp::Pipeline3D         pipeline;
    noisepp::ElementID          noise_id;
    noisepp::Cache*             noise_cache;
    noisepp::PipelineElement3D* perlin;

    double granularity;
    double rough_size;

    uint16_t rock_id;

    impl(world& w)
        : map           (w)
        , height_idx    (w.find_area_generator("heightmap"))
        , rough_idx     (w.find_area_generator("rough_terrain"))
        , surf_idx      (w.find_area_generator("surface"))
        , noise_cache   (nullptr)
    {
        if (height_idx < 0)
            throw std::runtime_error("standard_terrain_generator requires a height map");

        if (surf_idx < 0)
            throw std::runtime_error("standard_terrain_generator requires a surface map");

        noise.setOctaveCount(3);
        noise_id = noise.addToPipe(pipeline);
        perlin = pipeline.getElement(noise_id);
        noise_cache = pipeline.createCache();

        granularity = 40.;
        rough_size = 30.;

        rock_id  = find_material("stone");
    }

    ~impl()
    {
        pipeline.freeCache(noise_cache);
    }

    void generate(chunk_coordinates pos, chunk& dest)
    {
        trace("start standard terrain generation for %1%", world_vector(pos - world_chunk_center));

        if (   pos.x >= chunk_world_limit.x
            || pos.y >= chunk_world_limit.y
            || pos.z >= chunk_world_limit.z)
        {
            throw std::logic_error("not a valid chunk position");
        }

        world_coordinates offset (pos * chunk_size);
        area_ptr hm      (map.get_area_data(pos, height_idx));
        area_ptr rm      (nullptr);
        area_ptr sm      (nullptr);

        if (surf_idx >= 0)
            sm = map.get_area_data(pos, surf_idx);

        if (rough_idx >= 0)
            rm = map.get_area_data(pos, rough_idx);

        if (hm == nullptr)
            throw std::runtime_error("no height map was generated");

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
                    {
                        dest(x,y,z) = rock_id;
                        auto h16 (convert_height_16bit(offset.z + z));
                        if (sm && (*sm)(x,y) < h16)
                            (*sm)(x,y) = h16;

                    }
                }
            }
        }
        trace("end standard terrain generation for %1%", world_vector(pos - world_chunk_center));
    }

    chunk_height estimate_height (map_coordinates xy) const
    {
        area_ptr hm (map.get_area_data(xy, height_idx));
        if (hm == nullptr)
            return undefined_height;

        area_ptr rm      (nullptr);
        if (rough_idx >= 0)
            rm = map.get_area_data(xy, rough_idx);

        area_ptr sm      (nullptr);
        if (surf_idx >= 0)
        {
            sm = map.get_area_data(xy, surf_idx);
            if (sm == nullptr)
            {
                sm = std::make_shared<area_data>();
                constexpr int16_t undefined (std::numeric_limits<uint16_t>::max());
                fill(*sm, undefined);
            }
        }
        else
        {
            trace("no surface data");
        }

        uint32_t highest (0);
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
                if (rel_h < 50)
                    mul_b *= std::max((rel_h - 20.) / 30., 0.0);

                if (sm != nullptr)
                {
                    uint32_t h (world_center.z + rel_h);
                    if (mul_b == 0.0)
                    {
                        (*sm)(x,y) = rel_h;
                        highest = std::max(highest, h);
                    }
                    else
                    {
                        for (int z (rough_size); z > -rough_size; --z)
                        {
                            vector3<double> pn (xy.x * chunk_size + x , xy.y * chunk_size + y, h + z);
                            pn /= granularity;
                            double p (perlin->getValue(pn.x, pn.y, pn.z, noise_cache) * rough_size);

                            if (z + int(mul_b * p) <= 0)
                            {
                                (*sm)(x,y) = rel_h + z;
                                highest = std::max(highest, h + z);
                                break;
                            }
                        }
                    }
                }
                else
                {
                    uint32_t h (world_center.z + rel_h + rough_size * mul_b);
                    highest = std::max(highest, h);
                }
            }
        }

        if (surf_idx >= 0)
            map.store(xy, surf_idx, sm);

        return (highest / chunk_size) + 1;
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
standard_world_generator::estimate_height (map_coordinates xy, chunk_height prev) const
{
    return pimpl_->estimate_height(xy);
}

} // namespace hexa

