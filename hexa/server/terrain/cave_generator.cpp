//---------------------------------------------------------------------------
// server/terrain/cave_generator.cpp
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

#include "cave_generator.hpp"

#include <cassert>
#include <unordered_map>
#include <boost/math/constants/constants.hpp>

#include <hexa/block_types.hpp>
#include <hexa/container_uptr.hpp>
#include <hexa/lru_cache.hpp>
#include <hexa/trace.hpp>
#include <hexa/voxel_range.hpp>

#include "../random.hpp"
#include "../voxel_shapes.hpp"
#include "../world.hpp"

using namespace boost::math::constants;
using boost::property_tree::ptree;

namespace hexa
{

struct cave_generator::impl
{
    chunk_coordinates section_size;
    float vary_yaw;
    float vary_pitch;

    /** A section is a box of chunks.  The generator only creates one cave
     ** system per section.
     *
     *  The size of a section is determined by \a section_size. It is
     *  possible that caves of nearby sections spill over. */
    typedef vector3<uint32_t> cave_section;

    struct cave
    {
        /** The actual 3D shapes that make up the cave. */
        vector_uptr<csg::shape> parts;

        /** Map chunk coordinates to a list of the 3D shapes involved in
         ** that chunk. */
        std::unordered_map<chunk_coordinates, std::vector<size_t>> part_map;

#if defined(_MSC_VER)
        cave() {}
        cave(cave&& m)
            : parts(std::move(m.parts))
            , part_map(std::move(m.part_map))
        {
        }
        cave& operator=(cave&& m)
        {
            if (&m != this) {
                parts = std::move(m.parts);
                part_map = std::move(m.part_map);
            }
            return *this;
        }
#endif
    };

    /** Most recently used caves. */
    lru_cache<cave_section, cave> cache;

    impl(const ptree& conf)
        : section_size(8, 8, 4)
        , vary_yaw(conf.get<float>("vary_yaw", 1.2f))
        , vary_pitch(conf.get<float>("vary_pitch", 0.2f))
    {
    }

    ~impl() {}

    cave make_cave(cave_section pos) const
    {
        cave result;

        auto hash(fnv_hash(pos));
        auto start_pos(prng_next_pos(hash).mod(section_size));
        auto nr_corridors((prng_next(hash) % 4) + 1);

        for (uint32_t i(0); i < nr_corridors; ++i) {
            float total_length(0.0f);
            float corridor_length((prng_next(hash) % 80) + 50);
            vec3f pos(start_pos);
            float yaw(prng_next_f(hash) * pi<float>());
            yaw_pitch direction(yaw,
                                half_pi<float>() + 0.5f * prng_next_f(hash));
            float radius(prng_next_f(hash) + 4.0f);

            result.parts.emplace_back(
                std::make_unique<csg::sphere>(pos, radius));

            // Keep carving a corridor, randomly varying the direction and
            // radius
            // along the way, until we reach the desired length.
            //
            while (total_length < corridor_length) {
                float length(prng_next_f(hash) * 3.0f + 8.0f);
                float next_radius(
                    clamp(radius + prng_next_f(hash), 0.7f, 9.0f));
                vec3f next_pos(pos + from_spherical(direction) * length);

                result.parts.emplace_back(
                    std::make_unique<csg::sphere>(next_pos, next_radius));
                result.parts.emplace_back(
                    std::make_unique<csg::truncated_cone>(
                        pos, radius, next_pos, next_radius));

                radius = next_radius;
                pos = next_pos;
                direction.x += prng_next_f(hash) * vary_yaw;
                direction.y += prng_next_f(hash) * vary_pitch;

                // Don't go completely vertical.
                direction.y = clamp(direction.y, pi<float>() * 0.2f,
                                    pi<float>() * 0.8f);

                total_length += length;
            }
        }

        // Keep a map between chunks and shapes that intersect them.
        //
        chunk_coordinates offset(pos * section_size);
        for (size_t i(0); i < result.parts.size(); ++i) {
            auto& shape(*result.parts[i]);
            for (auto& j : shape.chunks())
                result.part_map[offset + j].emplace_back(i);
        }

        return result;
    }

    const cave& get_cave(cave_section pos)
    {
        auto cached(cache.try_get(pos));
        if (cached)
            return *cached;

        cache.prune(128);
        return cache[pos] = make_cave(pos);
    }

    void generate(world_terraingen_access& data, const chunk_coordinates& pos,
                  chunk& cnk)
    {
        cave_section csp(pos / section_size);
        for (auto i : surroundings(csp, 1)) {
            vec3f offset(world_vector(pos - (i * section_size)) * chunk_size);

            auto& cavesystem(get_cave(i));
            auto found(cavesystem.part_map.find(pos));
            if (found != cavesystem.part_map.end()) {
                for (auto k : every_block_in_chunk) {
                    auto& blk(cnk[k]);
                    if (blk == type::air)
                        continue;

                    vec3f blkpos(offset + vec3f(k));
                    for (auto index : found->second) {
                        auto& carve_shape(cavesystem.parts[index]);
                        if (carve_shape->is_inside(blkpos)) {
                            blk = type::air;
                            break;
                        }
                    }
                }
            }
        }
    }
};

//---------------------------------------------------------------------------

cave_generator::cave_generator(world& w, const ptree& conf)
    : terrain_generator_i(w)
    , pimpl_(new impl(conf))
{
}

cave_generator::~cave_generator()
{
}

void cave_generator::generate(world_terraingen_access& data,
                              const chunk_coordinates& pos, chunk& cnk)
{
    pimpl_->generate(data, pos, cnk);
}

} // namespace hexa
