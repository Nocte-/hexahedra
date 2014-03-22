//---------------------------------------------------------------------------
/// \file   hexa/unit_tests/test_terraingen.hpp
/// \brief  Unit tests for the server's terrain generation
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

#include <boost/test/unit_test.hpp>

#include <chrono>
#include <random>
#include <set>
#include <thread>
#include <boost/range/algorithm.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <hexanoise/generator_context.hpp>
#include <hexanoise/simple_global_variables.hpp>

#include <hexa/block_types.hpp>
#include <hexa/voxel_range.hpp>
#include <hexa/persistence_leveldb.hpp>
#include <hexa/server/init_terrain_generators.hpp>
#include <hexa/server/world.hpp>
#include <hexa/server/random.hpp>
#include <hexa/server/voxel_shapes.hpp>
#include <hexa/server/terrain/testpattern_generator.hpp>

using namespace hexa;

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;

// Sets up a world with a database, a noise generator context, and some
// global variables.
struct fixture
{
    fixture()
        : store ("test.db")
        , w (store)
        , ctx (vars)
    {
        vars["seed"] = 5.0;
        vars["one"] = 1.0;
        vars["two"] = 2.0;
    }

    ~fixture()
    {
        fs::remove_all("test.db");
    }

    void setup (const std::string& file)
    {
        pt::ptree config;
        pt::read_json(file, config);
        init_terrain_gen(w, config, ctx);
    }

    persistence_leveldb             store;
    world                           w;
    noise::simple_global_variables  vars;
    noise::generator_context        ctx;
};

bool pattern_sample (world_coordinates pos)
{
    chunk_coordinates cp (pos / chunk_size);
    chunk_index       bp (pos % chunk_size);

    auto hash (fnv_hash(cp));
    for (auto p : every_block_in_chunk)
    {
        if (p == bp)
            return hash & 0x1;

        hash = prng(hash);
    }
    throw std::runtime_error("pattern_sample");
}

void check_chunk_index (const chunk_index pos)
{
    BOOST_CHECK(pos.x >= 0);
    BOOST_CHECK(pos.y >= 0);
    BOOST_CHECK(pos.z >= 0);
    BOOST_CHECK(pos.x < chunk_size);
    BOOST_CHECK(pos.y < chunk_size);
    BOOST_CHECK(pos.z < chunk_size);
}

//---------------------------------------------------------------------------

BOOST_FIXTURE_TEST_SUITE(terrain, fixture)

BOOST_AUTO_TEST_CASE (setup_tests)
{
    register_new_material(1).name = "one";
    register_new_material(2).name = "two";
    register_new_material(3).name = "three";
    register_new_material(4).name = "four";
    register_new_material(5).name = "five";
}

//---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE (area_test)
{
    setup("terrain_test_1.json");

    auto proxy (w.acquire_read_access());
    auto idx   (w.find_area_generator("surface"));
    BOOST_CHECK(idx >= 0); // Can we create an area?

    map_coordinates spot (100, 100);

    // Empty before instantiation?
    BOOST_CHECK(!proxy.is_area_available(spot, idx));
    // Nothing stored in file yet?
    BOOST_CHECK(!store.is_available(persistent_storage_i::area,
                                     {spot.x,spot.y,(unsigned int)idx}));

    auto& area (proxy.get_area_data(spot, idx));
    // Area filled with the default values?
    BOOST_CHECK_EQUAL(area(0, 0),   -32768);
    BOOST_CHECK_EQUAL(area(15, 0),  -32768);
    BOOST_CHECK_EQUAL(area(15, 15), -32768);

    // Is it available now?
    BOOST_CHECK(proxy.is_area_available(spot, idx));
    // Marked with cache: true, is it in file?
    BOOST_CHECK(store.is_available(persistent_storage_i::area,
                                    {spot.x,spot.y,(unsigned int)idx}));
}

BOOST_AUTO_TEST_CASE (chunk_test)
{
    setup("terrain_test_2.json");

    auto proxy (w.acquire_read_access());
    // Can we fetch the coarse height? Is is correct?
    BOOST_CHECK_EQUAL(proxy.get_coarse_height({0,0}),   world_chunk_center.z);
    BOOST_CHECK_EQUAL(proxy.get_coarse_height({10,10}), world_chunk_center.z);
    // Get coarse height without triggering terrain gen?
    BOOST_CHECK(!proxy.is_chunk_available(world_chunk_center));

    auto pos (world_chunk_center - world_vector(0, 0, 1));
    auto& cnk (proxy.get_chunk(pos));
    // Terrain gen was triggered, is it available now?
    BOOST_CHECK(proxy.is_chunk_available(pos));
    BOOST_CHECK(store.is_available(persistent_storage_i::chunk, pos));

    // Does the chunk contain the values we expect?
    BOOST_CHECK_EQUAL(cnk(0,0,0), 1);
    BOOST_CHECK_EQUAL(cnk(15,0,0), 1);
    BOOST_CHECK_EQUAL(cnk(15,15,15), 1);

    // Does the chunk right above the waterline contain air?
    BOOST_CHECK_EQUAL(proxy.get_block(world_center), 0);
    // And right below the waterline, water?
    BOOST_CHECK_EQUAL(proxy.get_block(world_center - world_vector(0,0,1)), 1);
    // Check lower limit
    BOOST_CHECK_EQUAL(proxy.get_block({0,0,0}), 1);
    // Check upper limit
    BOOST_CHECK_EQUAL(proxy.get_block({0xffffffff, 0xffffffff, 0xffffffff}), 0);
}

BOOST_AUTO_TEST_CASE (surface_test_1)
{
    setup("terrain_test_3.json");
    auto& m (register_new_material(1));

    m.is_solid = true;
    m.transparency = 0;

    auto proxy (w.acquire_read_access());
    chunk_coordinates pos (10, 10, 10);
    auto& surf (proxy.get_surface(pos));

    // Building a surface should trigger terrain gen in the Neumann
    // neighborhood around pos.
    for (auto& c : neumann_neighborhood)
        BOOST_CHECK(proxy.is_chunk_available(pos + c));

    const chunk& cnk (proxy.get_chunk(pos));
    BOOST_CHECK_EQUAL(cnk(0,0,0), 1);
    BOOST_CHECK_EQUAL(cnk(0,0,1), 0);
    BOOST_CHECK_EQUAL(cnk(0,1,0), 0);
    BOOST_CHECK_EQUAL(cnk(1,0,0), 0);
    BOOST_CHECK_EQUAL(cnk(0,1,1), 1);
    BOOST_CHECK_EQUAL(cnk(1,1,0), 1);
    BOOST_CHECK_EQUAL(cnk(1,0,1), 1);
    BOOST_CHECK_EQUAL(cnk(1,1,1), 0);

    // Do we have a surface now?
    BOOST_CHECK(proxy.is_surface_available(pos));
    // No surface for the neighboring chunk?
    BOOST_CHECK(!proxy.is_surface_available(pos + world_vector(1, 0, 0)));

    // Checkerboard pattern: must have surfaces for exactly half the voxels
    BOOST_CHECK_EQUAL(surf.opaque.size(), chunk_volume / 2);
    // No transparent surfaces
    BOOST_CHECK_EQUAL(surf.transparent.size(), 0);

    // Make sure every surface element has the right type and six exposed
    // faces.
    for (auto& x : surf.opaque)
    {
        BOOST_CHECK_EQUAL(x.type, 1);
        BOOST_CHECK_EQUAL(x.dirs, 0x3f);
        BOOST_CHECK(manhattan_length(x.pos) % 2 == 0);
        check_chunk_index(x.pos);
    }
}

BOOST_AUTO_TEST_CASE (surface_test_2)
{
    // Very similar to surface_test_1, but using a psuedorandom pattern
    // instead of the checkerboard.

    setup("terrain_test_3.json");
    auto& m (register_new_material(1));

    m.is_solid = true;
    m.transparency = 0;

    auto proxy (w.acquire_read_access());
    chunk_coordinates pos (world_chunk_center.x + 10, 10, 10);
    auto& surf (proxy.get_surface(pos));

    for (auto& c : neumann_neighborhood)
        BOOST_CHECK(proxy.is_chunk_available(pos + c));

    BOOST_CHECK(proxy.is_surface_available(pos));
    BOOST_CHECK(!proxy.is_surface_available(pos + world_vector(1, 0, 0)));

    BOOST_CHECK_EQUAL(surf.transparent.size(), 0);

    for (auto& x : surf.opaque)
    {
        check_chunk_index(x.pos);
        world_coordinates wc (pos * chunk_size + x.pos);
        BOOST_CHECK(pattern_sample(wc));
        for (int i (0); i < 6; ++i)
        {
            BOOST_CHECK_EQUAL(!pattern_sample(wc + dir_vector[i]), x[i]);
        }
    }
}

//---------------------------------------------------------------------------


BOOST_AUTO_TEST_CASE (hndl_1_test)
{
    // Basic test for area generators; HNDL distance function

    setup("terrain_test_4.json");

    auto proxy (w.acquire_read_access());
    auto idx   (w.find_area_generator("distance_test"));

    map_coordinates spot (10, 10);
    auto& area (proxy.get_area_data(map_chunk_center + spot, idx));
    BOOST_CHECK_EQUAL(area(0, 0), 226);
}

BOOST_AUTO_TEST_CASE (hndl_2_test)
{
    // Test the @-operator in HNDL

    setup("terrain_test_4.json");

    auto proxy (w.acquire_read_access());
    auto idx   (w.find_area_generator("distance_div"));

    map_coordinates spot (10, 10);
    auto& area (proxy.get_area_data(map_chunk_center + spot, idx));
    BOOST_CHECK_EQUAL(area(0, 0), 23);
}

BOOST_AUTO_TEST_CASE (hndl_3_test)
{
    // Test global variables in HNDL

    setup("terrain_test_4.json");

    auto proxy (w.acquire_read_access());
    auto idx   (w.find_area_generator("global_test"));

    map_coordinates spot (10, 10);
    auto& area (proxy.get_area_data(map_chunk_center + spot, idx));
    BOOST_CHECK_EQUAL(area(0, 0), 2);
}

//---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE (hmgen_1_test)
{
    // Testing the heightmap terrain generator

    setup("terrain_test_5.json");

    auto proxy (w.acquire_read_access());
    auto idx   (w.find_area_generator("heightmap"));
    BOOST_CHECK(idx >= 0); // Can we create an area?

    // Make sure the height map is what we expect ("x:div(2):sub(0.49)")
    auto& area (proxy.get_area_data(map_chunk_center, idx));
    BOOST_CHECK_EQUAL(area(0, 0),   0);
    BOOST_CHECK_EQUAL(area(8, 0),   4);
    BOOST_CHECK_EQUAL(area(8, 8),   4);
    BOOST_CHECK_EQUAL(area(15, 15), 7);

    auto& area2 (proxy.get_area_data(map_chunk_center + vec2i(1,0), idx));
    BOOST_CHECK_EQUAL(area2(0, 0),   8);
    BOOST_CHECK_EQUAL(area2(15, 15), 15);

    auto& area3 (proxy.get_area_data(map_chunk_center + vec2i(2,0), idx));
    BOOST_CHECK_EQUAL(area3(0, 0),   16);
    BOOST_CHECK_EQUAL(area3(15, 15), 23);

    // Is the coarse height map calculated correctly?
    BOOST_CHECK_EQUAL(proxy.get_coarse_height(map_chunk_center),              world_chunk_center.z + 1);
    BOOST_CHECK_EQUAL(proxy.get_coarse_height(map_chunk_center + vec2i(1,0)), world_chunk_center.z + 1);
    BOOST_CHECK_EQUAL(proxy.get_coarse_height(map_chunk_center + vec2i(2,0)), world_chunk_center.z + 2);
    BOOST_CHECK_EQUAL(proxy.get_coarse_height(map_chunk_center + vec2i(3,0)), world_chunk_center.z + 2);

    // For chunks just below the coarse height map boundary, is the
    // boundary between air and material correct?
    auto& cnk (proxy.get_chunk(world_chunk_center));
    BOOST_CHECK_EQUAL(cnk(8,8,0),  1);
    BOOST_CHECK_EQUAL(cnk(8,8,3),  1);
    BOOST_CHECK_EQUAL(cnk(8,8,4),  0);
    BOOST_CHECK_EQUAL(cnk(8,8,15), 0);
}

//---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE (voxel_shape_1_test)
{
    csg::sphere sph ({3, 3, 3}, 6);
    BOOST_CHECK_EQUAL(sph.bounding_box(), aabb<vec3f>(-3, -3, -3, 9, 9, 9));

    auto sph_cnks (sph.chunks());
    BOOST_CHECK(sph_cnks.count({0,0,0}) == 1);
    BOOST_CHECK(sph_cnks.count({1,0,0}) == 0);
    BOOST_CHECK(sph_cnks.count({0,1,0}) == 0);
    BOOST_CHECK(sph_cnks.count({-1,0,0}) == 1);
    BOOST_CHECK(sph_cnks.count({0,-1,0}) == 1);
    BOOST_CHECK(sph_cnks.count({0,0,-1}) == 1);
    BOOST_CHECK(sph_cnks.count({-1,-1,-1}) == 1);

    BOOST_CHECK_EQUAL(csg::sphere({ 0, 0, 0}, 2).chunks().size(), 8);
    BOOST_CHECK_EQUAL(csg::sphere({ 8, 8, 8}, 9).chunks().size(), 7);
    BOOST_CHECK_EQUAL(csg::sphere({-8, 8, 8}, 9).chunks().size(), 7);
    BOOST_CHECK_EQUAL(csg::sphere({-8,-8, 8}, 9).chunks().size(), 7);
    BOOST_CHECK_EQUAL(csg::sphere({-8,-8,-8}, 9).chunks().size(), 7);
    BOOST_CHECK_EQUAL(csg::sphere({-24,-8,-8}, 9).chunks().size(), 7);
    BOOST_CHECK_EQUAL(csg::sphere({-16, 8, 8}, 7).chunks().size(), 2);

    csg::truncated_cone cone1 ({8, 8, 1}, 5, {8,8,15}, 2);
    BOOST_CHECK(cone1.is_inside({4,8,1}));
    BOOST_CHECK(cone1.is_inside({8,8,1}));
    BOOST_CHECK(cone1.is_inside({12,8,1}));

    BOOST_CHECK(cone1.is_inside({8,8,5}));

    BOOST_CHECK(cone1.is_inside({8,8,15}));
    BOOST_CHECK(cone1.is_inside({7,8,15}));
    BOOST_CHECK(cone1.is_inside({9,8,15}));

    BOOST_CHECK(!cone1.is_inside({2,8,1}));
    BOOST_CHECK(!cone1.is_inside({14,8,1}));

    BOOST_CHECK(!cone1.is_inside({8,3,5}));

    BOOST_CHECK(!cone1.is_inside({5,8,15}));
    BOOST_CHECK(!cone1.is_inside({11,8,15}));

    BOOST_CHECK(!cone1.is_inside({8,8,0}));
    BOOST_CHECK(!cone1.is_inside({8,8,16}));

}

//---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE (restore_test)
{
    setup("terrain_test_3.json");
    auto& m (register_new_material(1));
    m.is_solid = true;
    m.transparency = 0;
    std::unordered_map<chunk_coordinates, surface_data> srf;

    uint32_t mask (0xffffffff);
    mask >>= cnkshift;
    {
    auto proxy (w.acquire_read_access());
    uint32_t rng (666);
    for (int i (0); i < 10; ++i)
    {
        auto p (prng_next_pos(rng));
        p.x &= mask; p.y &= mask; p.z &= mask;
        srf[p] = proxy.get_surface(p);
    }
    }

    store.close();

    {
    persistence_leveldb         store2 ("test.db");
    world                       w2 (store2);
    testpattern_generator       gen (w2, {});

    uint32_t rng (666);
    auto proxy (w2.acquire_read_access());
    chunk tmpcnk;

    for (int i (0); i < 10; ++i)
    {
        auto p (prng_next_pos(rng));
        p.x &= mask; p.y &= mask; p.z &= mask;
        for (auto  j : neumann_neighborhood)
            BOOST_CHECK(proxy.is_chunk_available(p + j));

        BOOST_CHECK_EQUAL(proxy.get_coarse_height(p), chunk_world_limit.z);
        BOOST_CHECK(proxy.is_surface_available(p));
        BOOST_CHECK(proxy.get_surface(p) == srf[p]);
        BOOST_CHECK(proxy.is_chunk_available(p));
        gen.make_pattern(p, tmpcnk);
        BOOST_CHECK_EQUAL(proxy.get_chunk(p), tmpcnk);
    }
    }
}

//---------------------------------------------------------------------------

BOOST_AUTO_TEST_CASE (soil_test)
{
    setup("terrain_test_6.json");

    auto proxy (w.acquire_read_access());
    auto idx   (w.find_area_generator("biomes"));
    BOOST_CHECK(idx >= 0); // Do we have biomes?

    // Make sure the biome map is what we expect
    auto& bm (proxy.get_area_data(map_chunk_center, idx));
    BOOST_CHECK_EQUAL(bm(0, 0),  0);
    BOOST_CHECK_EQUAL(bm(1, 0),  1);
    BOOST_CHECK_EQUAL(bm(1, 8),  1);
    BOOST_CHECK_EQUAL(bm(4, 15), 4);

    auto idx2   (w.find_area_generator("surface"));
    BOOST_CHECK(idx2 >= 0);

    auto& cnk (proxy.get_chunk(world_chunk_center + world_vector(0,0,1)));
    BOOST_CHECK_EQUAL(cnk(0,0,4),  0);
    BOOST_CHECK_EQUAL(cnk(0,0,3),  3);
    BOOST_CHECK_EQUAL(cnk(0,0,2),  2);

    BOOST_CHECK_EQUAL(cnk(1,5,3),  4);
    BOOST_CHECK_EQUAL(cnk(1,5,2),  4);
    BOOST_CHECK_EQUAL(cnk(1,5,1),  2);

    BOOST_CHECK_EQUAL(cnk(2,5,3),  5);
    BOOST_CHECK_EQUAL(cnk(2,9,2),  4);
    BOOST_CHECK_EQUAL(cnk(2,8,1),  3);
    BOOST_CHECK_EQUAL(cnk(2,0,0),  2);

    BOOST_CHECK_EQUAL(cnk(3,5,3),  2);
    BOOST_CHECK_EQUAL(cnk(4,9,2),  2);

    auto& cnk2 (proxy.get_chunk(world_chunk_center));
    BOOST_CHECK_EQUAL(cnk2(2,5,15),  2);
    BOOST_CHECK_EQUAL(cnk2(0,0,15),  2);
    BOOST_CHECK_EQUAL(cnk2(1,7,15),  2);

}

BOOST_AUTO_TEST_SUITE_END()

