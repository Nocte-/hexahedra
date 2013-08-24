//---------------------------------------------------------------------------
/// \file   server/generic_area_generator.hpp
/// \brief  Simple 2-D Perlin generator.
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

#pragma once

#include "area_generator_i.hpp"

#include <noisepp/NoisePerlin.h>
#include <noisepp/NoisePipeline.h>
#include <hexa/basic_types.hpp>

namespace hexa {

/** Simple 2-D Perlin generator.
 *  The Swiss army knife of area generators.  You can chain a couple of 
 *  these to quickly whip up some terrain modifiers, biomes, etc. */
class generic_area_generator : public area_generator_i
{
public:
    generic_area_generator(world& w, std::string name, 
                           double spread, int octaves = 3,
                           double range = 20000.)
        : area_generator_i (w, name)
        , noise_cache_ (nullptr)
        , spread_      (spread)
        , range_       (range)
    {
        noise_.setOctaveCount(octaves);
        noise_id_ = noise_.addToPipe(pipeline_);
        perlin_ = pipeline_.getElement(noise_id_);
        noise_cache_ = pipeline_.createCache();
    }


    virtual ~generic_area_generator() 
    {
        pipeline_.freeCache(noise_cache_);
    }

    virtual area_data& 
    generate (map_coordinates xy, area_data& dest) 
    {
        assert(xy.x < chunk_world_limit.x);
        assert(xy.y < chunk_world_limit.y);

        const map_coordinates o (xy * chunk_size);
        for (uint32_t y (o.y); y < o.y + chunk_size; ++y)
        {
            for (uint32_t x (o.x); x < o.x + chunk_size; ++x)
            {
                double noise (perlin_->getValue(x / spread_, y / spread_, noise_cache_));
                dest(x - o.x, y - o.y) = static_cast<int16_t>(noise * range_);
            }   
        }

        return dest;
    }

private:
    noisepp::PerlinModule       noise_;
    noisepp::Pipeline2D         pipeline_;
    noisepp::ElementID          noise_id_;
    noisepp::Cache*             noise_cache_;
    noisepp::PipelineElement2D* perlin_;

    double                      spread_;
    double                      range_;
};

} // namespace hexa

