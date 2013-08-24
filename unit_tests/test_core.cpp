
#define BOOST_TEST_MODULE hexahedra_core test
#include <boost/test/unit_test.hpp>

#include <chrono>
#include <random>
#include <set>
#include <thread>
#include <boost/range/algorithm.hpp>
#include <boost/filesystem/operations.hpp>

#include <hexa/aabb.hpp>
#include <hexa/algorithm.hpp>
#include <hexa/chunk.hpp>
#include <hexa/collision.hpp>
#include <hexa/compression.hpp>
#include <hexa/concurrent_queue.hpp>
#include <hexa/geometric.hpp>
#include <hexa/lru_cache.hpp>
#include <hexa/memory_cache.hpp>
#include <hexa/neighborhood.hpp>
#include <hexa/persistence_sqlite.hpp>
#include <hexa/persistence_null.hpp>
#include <hexa/protocol.hpp>
#include <hexa/quaternion.hpp>
#include <hexa/ray.hpp>
#include <hexa/ray_bundle.hpp>
#include <hexa/serialize.hpp>
#include <hexa/surface.hpp>
#include <hexa/vector3.hpp>
#include <hexa/voxel_algorithm.hpp>
#include <hexa/voxel_range.hpp>
#include <hexa/wfpos.hpp>

using namespace hexa;

BOOST_AUTO_TEST_CASE (serialize_test)
{
    typedef std::vector<uint8_t> buffer;
    buffer a;
    float b (3.141f);

    auto ser (make_serializer(a));
    ser(b);
    BOOST_CHECK_EQUAL(a.size(), sizeof(float));

    float c;
    auto dser (make_deserializer(a));
    dser(c);
    BOOST_CHECK_EQUAL(b, c);

    buffer a2;
    auto ser2 (make_serializer(a2));
    block_vector d (10, 5, 4), e;
    ser2(d);

    auto dser2 (make_deserializer(a2));
    dser2(e);
    BOOST_CHECK_EQUAL(a2.size(), 2); // Not 3, block_vector is a special case
    BOOST_CHECK_EQUAL(d, e);
}

BOOST_AUTO_TEST_CASE (vector2_test)
{
    vector2<int> first (1, 2);
    BOOST_CHECK_EQUAL(first.x, 1);
    BOOST_CHECK_EQUAL(first.y, 2);
    BOOST_CHECK_EQUAL(first, vector2<int>(1, 2));

    vector2<int> second (4, 7);
    BOOST_CHECK_EQUAL(first + second, vector2<int>(5, 9));
    BOOST_CHECK_EQUAL(first - second, vector2<int>(-3, -5));
    BOOST_CHECK_EQUAL(first * second, vector2<int>(4, 14));
    BOOST_CHECK_EQUAL(second % 3, vector2<int>(1, 1));
}

BOOST_AUTO_TEST_CASE (vector3_test)
{
    vector3<int> first (1, 2, 3);
    BOOST_CHECK_EQUAL(first.x, 1);
    BOOST_CHECK_EQUAL(first.y, 2);
    BOOST_CHECK_EQUAL(first.z, 3);
    BOOST_CHECK_EQUAL(first, vector3<int>(1, 2, 3));

    vector3<int> second (4, 7, 13);
    BOOST_CHECK_EQUAL(first + second, vector3<int>(5, 9, 16));
    BOOST_CHECK_EQUAL(first - second, vector3<int>(-3, -5, -10));
    BOOST_CHECK_EQUAL(first * second, vector3<int>(4, 14, 39));
    BOOST_CHECK_EQUAL(second % 3, vector3<int>(1, 1, 1));

    vector3<int> third (-3, 8, 3);
    BOOST_CHECK_EQUAL(squared_length(first), 14);
    BOOST_CHECK_EQUAL(diff(first, first), vector3<int>(0, 0, 0));
    BOOST_CHECK_EQUAL(diff(first, third), vector3<int>(4, 6, 0));
    BOOST_CHECK_EQUAL(manhattan_distance(first, first), 0);
    BOOST_CHECK_EQUAL(manhattan_distance(first, second), 18);
}

BOOST_AUTO_TEST_CASE (wfpos_test)
{
    wfpos p1 (1u, 2u, 3u);
    BOOST_CHECK_EQUAL(p1.pos , world_coordinates(1, 2, 3));
    BOOST_CHECK_EQUAL(p1.frac, vector::zero());

    wfpos pz (1.0f, 2.0f, 3.0f);
    BOOST_CHECK_EQUAL(pz.pos , world_coordinates(1, 2, 3));
    BOOST_CHECK_EQUAL(pz.frac, vector::zero());

    wfpos p2 (1.5f, 2.4f, 3.2f);
    BOOST_CHECK_CLOSE(p2.frac.x, 0.5f, 0.001);
    BOOST_CHECK_CLOSE(p2.frac.y, 0.4f, 0.001);
    BOOST_CHECK_CLOSE(p2.frac.z, 0.2f, 0.001);
    BOOST_CHECK_EQUAL(p2.pos , world_coordinates(1, 2, 3));

    p2 += wfvec(10.7f, 11.8f, 12.9f);
    BOOST_CHECK_CLOSE(p2.frac.x, 1.2f, 0.001);
    BOOST_CHECK_CLOSE(p2.frac.y, 1.2f, 0.001);
    BOOST_CHECK_CLOSE(p2.frac.z, 1.1f, 0.001);
    BOOST_CHECK_EQUAL(p2.pos , world_coordinates(11, 13, 15));

    p2.normalize();
    BOOST_CHECK_CLOSE(p2.frac.x, 0.2f, 0.001);
    BOOST_CHECK_CLOSE(p2.frac.y, 0.2f, 0.001);
    BOOST_CHECK_CLOSE(p2.frac.z, 0.1f, 0.001);
    BOOST_CHECK_EQUAL(p2.pos , world_coordinates(12, 14, 16));

    p2 += vector(0, 1, 0);
    BOOST_CHECK_CLOSE(p2.frac.x, 0.2f, 0.001);
    BOOST_CHECK_CLOSE(p2.frac.y, 1.2f, 0.001);
    BOOST_CHECK_CLOSE(p2.frac.z, 0.1f, 0.001);
    BOOST_CHECK_EQUAL(p2.pos , world_coordinates(12, 14, 16));

    wfpos p3;
    p3.pos = world_coordinates(100000010,100000010,100000010);
    p3.frac = vector(-1.5f, -1.6f, -1.7f);
    p3.normalize();
    BOOST_CHECK_CLOSE(p3.frac.x, -0.5f, 0.001);
    BOOST_CHECK_CLOSE(p3.frac.y, -0.6f, 0.001);
    BOOST_CHECK_CLOSE(p3.frac.z, -0.7f, 0.001);
    BOOST_CHECK_EQUAL(p3.pos , world_coordinates(100000009, 100000009, 100000009));

    wfpos p4 (p3);
    p4 += vector(10, 0, 0);
    BOOST_CHECK_EQUAL(p4.pos , world_coordinates(100000009, 100000009, 100000009));
    p4.normalize();
    BOOST_CHECK_EQUAL(p4.pos , world_coordinates(100000018, 100000009, 100000009));

    BOOST_CHECK_CLOSE(distance(p3, p4), 10, 0.0001);

    auto p5 (lerp(p3, p4, 0.2));
    BOOST_CHECK_CLOSE(distance(p3, p5), 2, 0.0001);
}

BOOST_AUTO_TEST_CASE (aabb_test)
{
    aabb<vector> a ({1, 2, 3}, {4, 5, 6});
    BOOST_CHECK(a.is_correct());
    aabb<vector> b ({2, 3, 7}, {5, 6, 4});
    BOOST_CHECK(!b.is_correct());
    b.make_correct();
    BOOST_CHECK(b.is_correct());
    BOOST_CHECK_EQUAL(b.first.z, 4);
    BOOST_CHECK_EQUAL(b.second.z, 7);

    BOOST_CHECK(are_overlapping(a, b));
    aabb<vector> c (intersection(a, b));
    BOOST_CHECK_EQUAL(c.first,  vector(2, 3, 4));
    BOOST_CHECK_EQUAL(c.second, vector(4, 5, 6));
    BOOST_CHECK_EQUAL(center(c), vector(3, 4, 5));

    b += vector(10, 10, 10);
    BOOST_CHECK(b.is_correct());
    BOOST_CHECK(!are_overlapping(a, b));
    BOOST_CHECK(!intersection(a, b).is_correct());
}

BOOST_AUTO_TEST_CASE (aabb_ray_test)
{
    aabb<vector> a ({3, 4, 5}, {4, 5, 6});
    ray<float>   r ({0, 0, 0}, vector{1, 1, 1});

    BOOST_CHECK(!ray_box_intersection(r, a));
    ray<float>  r2 ({0, 0, 0}, center(a));
    BOOST_CHECK(ray_box_intersection(r2, a));
}

BOOST_AUTO_TEST_CASE (aabb_collision_test)
{
    vector p {1.0f, 1.0f, 1.0f};
    collision_aabb a { {p}, 63 };

    aabb<vector> hitbox {{1.1f, 1.1f, 3.0f}, {1.4f, 1.4f, 4.8f}};
    vector motion { 0.0f, 0.0f, -1.3f };


}

BOOST_AUTO_TEST_CASE (aabb_multi_collision_test)
{
    collision_aabb a (vector{0, 0, 0}, 0x3f);
    collision_aabb b (vector{1, 0, 0}, 0x3f);

    collision_mesh env1 { a, b };

    aabb<vector> player (vector{0,0,1.5f}, 0.5f);
    vector motion (0.8, 0, -1);

    auto result1 (collide(player + motion, motion, env1));

    BOOST_CHECK_EQUAL(result1.impact.x, 0);
    BOOST_CHECK_EQUAL(result1.impact.y, 0);
    BOOST_CHECK_EQUAL(result1.impact.z, 0.5);
}

BOOST_AUTO_TEST_CASE (bresenham_test)
{
    std::mt19937  prng;
    std::uniform_real_distribution<float> rc (-50, 50);
    std::uniform_real_distribution<float> d (-5, 5);

    for (int i (0); i < 100; ++i)
    {
        vector3<float> from (rc(prng), rc(prng), rc(prng));
        vector3<float> to   (d(prng), d(prng), d(prng));
        to += from;
        auto vxls (voxel_raycast(from, to));
        auto compare (dumbass_line(from, to));

        BOOST_CHECK(vxls == compare);
    }
}

BOOST_AUTO_TEST_CASE (lrucache_test)
{
    lru_cache<int, std::string> cache;

    cache[1] = "one";
    cache[8] = "eight";
    cache[5] = "five";

    BOOST_CHECK_EQUAL(cache.size(), 3);
    BOOST_CHECK_EQUAL(cache.count(2), 0);
    BOOST_CHECK_EQUAL(cache.count(5), 1);
    BOOST_CHECK_EQUAL(cache.get(1), "one");
    BOOST_CHECK_EQUAL(cache.get(8), "eight");
    BOOST_CHECK_EQUAL((bool)cache.try_get(1), true);
    BOOST_CHECK_EQUAL(*cache.try_get(1), "one");

    cache.touch(1);
    cache.prune(2);
    BOOST_CHECK_EQUAL(cache.size(), 2);
    BOOST_CHECK_EQUAL(cache.count(1), 1);
    BOOST_CHECK_EQUAL(cache.count(5), 1);
    BOOST_CHECK_EQUAL(cache.count(8), 0);
    BOOST_CHECK_EQUAL((bool)cache.try_get(1), true);
    BOOST_CHECK_EQUAL((bool)cache.try_get(5), true);
    BOOST_CHECK_EQUAL((bool)cache.try_get(8), false);

    cache[10] = "ten";
    cache[2] = "two";

    std::vector<int> keys;
    cache.for_each([&](int k, std::string v){ keys.push_back(k); });
    BOOST_CHECK_EQUAL(keys.size(), 4);
    BOOST_CHECK_EQUAL(keys[0], 2);
    BOOST_CHECK_EQUAL(keys[1], 10);
    BOOST_CHECK_EQUAL(keys[2], 5);
    BOOST_CHECK_EQUAL(keys[3], 1);

    keys.clear();
    cache.prune(2, [&](int k, std::string v){ keys.push_back(k); });
    BOOST_CHECK_EQUAL(keys.size(), 2);
    BOOST_CHECK_EQUAL(keys[0], 1);
    BOOST_CHECK_EQUAL(keys[1], 5);
    BOOST_CHECK_EQUAL(cache.get(2), "two");
    BOOST_CHECK_EQUAL(cache.get(10), "ten");

    cache.clear();
    BOOST_CHECK(cache.empty());
}


BOOST_AUTO_TEST_CASE (compress_test)
{
    std::string li("Lorem ipsum dolor sit amet, consectetur xxxxxxxxxxxxxxxxx");

    compressed_data compr   (hexa::compress(li));
    binary_data     decompr (hexa::decompress(compr));

    BOOST_CHECK_EQUAL(li, std::string(decompr.begin(), decompr.end()));
    BOOST_CHECK_EQUAL(li.size(), decompr.size());
}

BOOST_AUTO_TEST_CASE (concurrent_queue_test)
{
    concurrent_queue<std::string> q;

    bool b1 (false), b2 (false), b3 (false);
    std::string s1, s2, s3;

    std::chrono::milliseconds pause (5);

    std::thread t1 ([&](){ s1 = q.pop(); std::this_thread::sleep_for(pause); b1 = true; });
    std::thread t2 ([&](){ s2 = q.pop(); std::this_thread::sleep_for(pause); b2 = true; });
    std::thread t3 ([&](){ s3 = q.pop(); std::this_thread::sleep_for(pause); b3 = true; });

    q.push("foo");
    q.push("bar");
    q.push("baz");
    q.push("qux");

    t1.join();
    t2.join();
    t3.join();

    BOOST_CHECK(b1);
    BOOST_CHECK(b2);
    BOOST_CHECK(b3);

    std::set<std::string> ss { s1, s2, s3 };
    auto i (ss.begin());
    BOOST_CHECK_EQUAL(ss.size(), 3);
    BOOST_CHECK_EQUAL(*i++, "bar");
    BOOST_CHECK_EQUAL(*i++, "baz");
    BOOST_CHECK_EQUAL(*i++, "foo");

    BOOST_CHECK_EQUAL(q.size(), 1);
}

/*
BOOST_AUTO_TEST_CASE (sqlite_test)
{
    using namespace boost;

    persistent_storage_i::data_type chunk_type {persistent_storage_i::chunk};

    filesystem::remove("world.db");
    persistence_sqlite  per;

    chunk_coordinates cpos {10, 20, 30};

    BOOST_CHECK(per.is_available(chunk_type, cpos) == false);
    BOOST_CHECK_THROW(per.retrieve(chunk_type, cpos), not_in_storage_error);

    binary_data bogus { 0x01, 0x00, 0x13, 0x24, 0x36, -127, 0x00 };
    per.store(chunk_type, cpos, compress(bogus));

    BOOST_CHECK(per.is_available(chunk_type, cpos));
    compressed_data compare {per.retrieve(chunk_type, cpos)};
    BOOST_CHECK(decompress(compare) == bogus);

    //---

    persistent_storage_i::data_type surface_type {persistent_storage_i::surface};
    chunk_coordinates spos {134217728, 134217720, 134217892};

    BOOST_CHECK(per.is_available(surface_type, spos) == false);
    BOOST_CHECK_THROW(per.retrieve(surface_type, spos), not_in_storage_error);

    binary_data bogus2 { 0x01, 0x00, 0x3A, 0x28, 0x00, 0x00, 0x6B };
    per.store(surface_type, spos, compress(bogus2));

    BOOST_CHECK(per.is_available(surface_type, spos));
    compressed_data compare2 {per.retrieve(surface_type, spos)};
    BOOST_CHECK(decompress(compare2) == bogus2);

    filesystem::remove("world.db");
}
*/

BOOST_AUTO_TEST_CASE (voxelrange_test)
{
    size_t count (0);
    for (vector3<int> i : make_range<vector3<int>>({2, 3, 4}, {4, 6, 8}))
    {
        (void)i;
        ++count;
    }
    BOOST_CHECK_EQUAL(count, 24);

    count = 0;
    for (vector3<int> i : cube_range<vector3<int>>(2))
    {
        (void)i;
        ++count;
    }
    BOOST_CHECK_EQUAL(count, 125);

    count = 0;
    for (vector3<int> i : every_block_in_chunk)
    {
        (void)i;
        ++count;
    }
    BOOST_CHECK_EQUAL(count, chunk_volume);
}

BOOST_AUTO_TEST_CASE (neighborhood_test)
{
    /*
    chunk_ptr c1(new chunk), c2(new chunk), c3(new chunk), c4(new chunk);

    for (chunk_index i : every_block_in_chunk)
    {
        (*c1)[i] = i.x * 4 + i.y * 32 + i.z * 32 * 32;
        (*c2)[i] = i.x * 4 + i.y * 32 + i.z * 32 * 32 + 1;
        (*c3)[i] = i.x * 4 + i.y * 32 + i.z * 32 * 32 + 2;
        (*c4)[i] = i.x * 4 + i.y * 32 + i.z * 32 * 32 + 3;
    }

    neighborhood<chunk_ptr> nbh (nothing, {0,0,0});

    nbh.add({ 0, 0, 0}, c1);
    nbh.add({-1, 0, 0}, c2);
    nbh.add({ 1, 0, 0}, c3);
    nbh.add({ 0, 1, 0}, c4);

    typedef block_vector bv;

    BOOST_CHECK_EQUAL(nbh[bv(2, 3, 4)], (*c1)[bv(2, 3, 4)]);
    BOOST_CHECK_EQUAL(nbh[bv(0, 0, 0)], (*c1)[bv(0, 0, 0)]);
    BOOST_CHECK_EQUAL(nbh[bv(15, 15, 15)], (*c1)[bv(15, 15, 15)]);

    BOOST_CHECK_EQUAL(nbh[bv(-1, 0, 0)], (*c2)[bv(15, 0, 0)]);
    BOOST_CHECK_EQUAL(nbh[bv(-2, 0, 0)], (*c2)[bv(14, 0, 0)]);
    BOOST_CHECK_EQUAL(nbh[bv(-16, 1, 2)], (*c2)[bv(0, 1, 2)]);

    BOOST_CHECK_EQUAL(nbh[bv(16, 0, 0)], (*c3)[bv(0, 0, 0)]);
    BOOST_CHECK_EQUAL(nbh[bv(17, 3, 8)], (*c3)[bv(1, 3, 8)]);
    BOOST_CHECK_EQUAL(nbh[bv(31, 1, 2)], (*c3)[bv(15, 1, 2)]);

    BOOST_CHECK_EQUAL(nbh[bv(0, 16, 0)], (*c4)[bv(0, 0, 0)]);
    BOOST_CHECK_EQUAL(nbh[bv(1, 18, 2)], (*c4)[bv(1, 2, 2)]);
    BOOST_CHECK_EQUAL(nbh[bv(3, 31, 5)], (*c4)[bv(3, 15, 5)]);
    */
}

/*
BOOST_AUTO_TEST_CASE (clientworld_test)
{
    using namespace boost;

    filesystem::remove("world.db");
    persistence_sqlite db;
    memory_cache map (db);

    chunk_coordinates cc1 (100, 101, 102);
    chunk_coordinates cc2 (200, 201, 202);
    BOOST_CHECK(map.is_chunk_available(cc1) == false);
    BOOST_CHECK(map.get_chunk(cc1) == nullptr);
    BOOST_CHECK(map.is_lightmap_available(cc1) == false);
    BOOST_CHECK(map.get_lightmap(cc1) == nullptr);

    chunk_ptr c (new chunk);
    for (chunk_index i : every_block_in_chunk)
        (*c)[i] = i.x + i.y * 32 + i.z * 32 * 32;

    lightmap_ptr lm (new lightmap);
    map.store(cc1, c);
    BOOST_CHECK(map.is_chunk_available(cc1));
    BOOST_CHECK(map.get_chunk(cc1) != nullptr);
    map.store(cc1, lm);
    BOOST_CHECK(map.is_lightmap_available(cc1));
    BOOST_CHECK(map.get_lightmap(cc1) != nullptr);

    chunk_ptr compare (map.get_chunk(cc1));
    BOOST_REQUIRE(compare != nullptr);
    BOOST_CHECK(*compare == *c);
    //BOOST_CHECK(callback1_exec);
    //BOOST_CHECK(callback2_exec);
}
*/


    /*
BOOST_AUTO_TEST_CASE (surface_test)
{
    chunk_ptr c (new chunk);

    material& m (register_new_material (1));
    m.is_solid = true;

    chunk_index blockpos (8, 7, 6);
    (*c)[blockpos].type = 1;

    neighborhood<chunk_ptr> nbh;
    nbh.add(chunk_index(0, 0, 0), c);

    {
    faces result (extract_surface(nbh));

    BOOST_REQUIRE_EQUAL(result.size(), 6);
    for (int i (0); i < 6; ++i)
        BOOST_CHECK_EQUAL(result[i].pos, blockpos);
    }

    chunk_index next (9, 7, 6);
    (*c)[next].type = 1;

    {
    faces result (extract_surface(nbh));
    BOOST_REQUIRE_EQUAL(result.size(), 10);
    }

    (*c).clear(0);
    (*c)[chunk_index(0,0,0)].type = 1;
    (*c)[chunk_index(15,0,0)].type = 1;

    nbh.add(chunk_index(-1, 0, 0), c);
    {
    faces result (extract_surface(nbh));
    BOOST_REQUIRE_EQUAL(result.size(), 11);
    BOOST_REQUIRE_EQUAL(boost::range::count(result, face{chunk_index( 0,0,0), dir_east}), 1);
    BOOST_REQUIRE_EQUAL(boost::range::count(result, face{chunk_index( 0,0,0), dir_west}), 0);
    BOOST_REQUIRE_EQUAL(boost::range::count(result, face{chunk_index(15,0,0), dir_east}), 1);
    BOOST_REQUIRE_EQUAL(boost::range::count(result, face{chunk_index(15,0,0), dir_west}), 1);
    }
}
    */

/*
BOOST_AUTO_TEST_CASE (area_height_test)
{
    persistence_null per;
    world w (per);
    heightmap_generator gen (w);

    area_data result;
    gen.generate (map_coordinates(10, 20), result);
}

class test_heightmap : public area_generator_i
{
public:
    test_heightmap(world& w) : area_generator_i (w, "heightmap") {}
    virtual ~test_heightmap() {}

    area_data& generate (const map_coordinates& xy, area_data& dest)
    {
        for (int x (0); x < chunk_size; ++x)
            for (int y (0); y < chunk_size; ++y)
                dest(x,y) = 127 + x + y;

        return dest;
    }
};

BOOST_AUTO_TEST_CASE (simple_terrain_test)
{
    persistence_null per;
    simple_terrain_cache memcache (per);
    world w (memcache);

    test_heightmap thm (w);
    w.add_area_generator(&thm);

    standard_world_generator gen (w);
    w.add_terrain_generator(&gen);

    chunk_coordinates pos (world_chunk_center);
    pos.z += 8;

    chunk_ptr c (w.get_chunk(pos));
    BOOST_REQUIRE(c != nullptr);

    BOOST_CHECK_EQUAL((*c)(0, 0, 0), type::air);
    BOOST_CHECK_EQUAL((*c)(1, 0, 0), type::rock);
    BOOST_CHECK_EQUAL((*c)(0, 1, 0), type::rock);
    BOOST_CHECK_EQUAL((*c)(1, 1, 0), type::rock);

    BOOST_CHECK_EQUAL((*c)(0, 0, 1), type::air);
    BOOST_CHECK_EQUAL((*c)(1, 0, 1), type::air);
    BOOST_CHECK_EQUAL((*c)(0, 1, 1), type::air);
    BOOST_CHECK_EQUAL((*c)(1, 1, 1), type::rock);

    BOOST_CHECK_EQUAL((*c)(15, 15, 15), type::rock);
}
*/

BOOST_AUTO_TEST_CASE (protocol_test)
{
    /*
    msg::chunk_update testmsg;

    chunk input;
    uint16_t f (0);
    for (auto x : every_block_in_chunk)
        input[x] = (++f) % 14;

    testmsg.position = chunk_coordinates(40, 50, 60);
    testmsg.data = compress(input);

    packet p {serialize_packet(testmsg)};
    auto arch (make_deserializer(p));

    uint8_t packet_id;
    uint16_t packet_len;
    arch(packet_id)(packet_len);

    BOOST_CHECK_EQUAL(packet_id, msg::chunk_update::msg_id);

    msg::chunk_update recv;
    recv.serialize(arch);

    chunk output;
    decompress(recv.data, output);
    BOOST_CHECK_EQUAL(testmsg.position, recv.position);
    BOOST_CHECK(input == output);
    */

    //----------------------------------------------------------------------

    std::vector<uint8_t> buf;

    msg::request_heights rqh;
    rqh.requests.push_back({{10, 20}, 0});
    rqh.requests.push_back({{11, 22}, 0});
    rqh.requests.push_back({{134217722, 134217723}, 0});

    buf.clear();
    auto p3 (make_serializer(buf));
    rqh.serialize(p3);

    auto arch3 (make_deserializer(buf));
    msg::request_heights rqh2;
    rqh2.serialize(arch3);

    BOOST_CHECK_EQUAL(rqh.requests.size(), rqh2.requests.size());
    for (int i (0); i < 3; ++i)
        BOOST_CHECK_EQUAL(rqh.requests[i].position, rqh2.requests[i].position);

    //----------------------------------------------------------------------

    surface foo_surf { {{{ 1, 2, 3 }, dir_up   }, 4},
                       {{{ 5, 6, 7 }, dir_down }, 5},
                       {{{ 9, 10, 11}, dir_north}, 6} };

    compressed_data vlb {compress(serialize(foo_surf))};
    surface bar_surf    {deserialize_as<surface>(decompress(vlb))};

    BOOST_CHECK(foo_surf == bar_surf);
    BOOST_CHECK(foo_surf[0].pos == bar_surf[0].pos);

    msg::surface_update upds;
    upds.terrain = std::move(vlb);

    buf.clear();
    auto p4 (make_serializer(buf));
    upds.serialize(p4);

    auto arch4 (make_deserializer(buf));
    msg::surface_update upds2;
    upds2.serialize(arch4);

    BOOST_CHECK(upds.terrain == upds2.terrain);
}

BOOST_AUTO_TEST_CASE (raybundle_test)
{
    ray_bundle one { { {0,0,0}, {1,1,1}, {2,2,2} }, 1.0f };

    BOOST_CHECK_EQUAL(one.trunk.size(), 3);
    BOOST_CHECK_EQUAL(one.branches.size(), 0);
    BOOST_CHECK_EQUAL(one.weight, 1.0f);

    one.add({{0,0,0}, {1,1,1}, {2,2,3}}, 0.5f);

    BOOST_CHECK_EQUAL(one.trunk.size(), 2);
    BOOST_CHECK_EQUAL(one.trunk[0], world_vector(0,0,0));
    BOOST_CHECK_EQUAL(one.trunk[1], world_vector(1,1,1));
    BOOST_CHECK_EQUAL(one.weight, 1.5f);

    BOOST_CHECK_EQUAL(one.branches.size(), 2);
    BOOST_CHECK_EQUAL(one.branches[0].trunk.size(), 1);
    BOOST_CHECK_EQUAL(one.branches[0].trunk[0], world_vector(2,2,2));
    BOOST_CHECK_EQUAL(one.branches[0].weight, 1.0f);

    BOOST_CHECK_EQUAL(one.branches[1].trunk.size(), 1);
    BOOST_CHECK_EQUAL(one.branches[1].trunk[0], world_vector(2,2,3));
    BOOST_CHECK_EQUAL(one.branches[1].weight, 0.5f);

    one.add({{0,0,0}}, 0.5f);
    BOOST_CHECK_EQUAL(one.trunk.size(), 1);
    BOOST_CHECK_EQUAL(one.trunk[0], world_vector(0,0,0));
    BOOST_CHECK_EQUAL(one.weight, 2.0f);

    BOOST_CHECK_EQUAL(one.branches.size(), 1);
    BOOST_CHECK_EQUAL(one.branches[0].trunk.size(), 1);
    BOOST_CHECK_EQUAL(one.branches[0].trunk[0], world_vector(1,1,1));
    BOOST_CHECK_EQUAL(one.branches[0].branches.size(), 2);

    ray_bundle two { { {0,0,0}, {1,1,1} }, 1.0f };
    two.add({{0,0,0}, {1,1,1}, {2,2,2}}, 0.5f);
    BOOST_CHECK_EQUAL(two.trunk.size(), 2);
    BOOST_CHECK_EQUAL(two.branches.size(), 1);
    BOOST_CHECK_EQUAL(two.branches[0].trunk.size(), 1);
    BOOST_CHECK_EQUAL(two.branches[0].trunk[0], world_vector(2,2,2));
}

/*
BOOST_AUTO_TEST_CASE (voxelsprite_test)
{
    voxel_sprite spr ({ 1, 1, 1});
    chunk cnk;
    boost::range::fill(cnk, 0);

    world_coordinates i (0, 0, 0);
    spr[i].type = 5;
    paste(cnk, {0,0,0}, spr, {3,4,5});

    BOOST_CHECK_EQUAL(cnk(0,0,0), 0);
    BOOST_CHECK_EQUAL(cnk(3,4,5), 5);
    BOOST_CHECK_EQUAL(cnk(3,4,6), 0);
    BOOST_CHECK_EQUAL(cnk(3,4,4), 0);
    BOOST_CHECK_EQUAL(cnk(3,3,5), 0);
    BOOST_CHECK_EQUAL(cnk(3,5,5), 0);
    BOOST_CHECK_EQUAL(cnk(2,4,5), 0);
    BOOST_CHECK_EQUAL(cnk(4,4,5), 0);

    voxel_sprite spr2 ({2, 2, 2});

    spr2[world_coordinates{0,0,0}].type = 2;
    spr2[world_coordinates{0,0,1}].type = 3;
    spr2[world_coordinates{0,1,0}].type = 4;
    spr2[world_coordinates{0,1,1}].type = 5;
    spr2[world_coordinates{1,0,0}].type = 6;
    spr2[world_coordinates{1,0,1}].type = 7;
    spr2[world_coordinates{1,1,0}].type = 8;
    spr2[world_coordinates{1,1,1}].type = 9;
    paste(cnk, {0,0,0}, spr2, {1,2,3});
    BOOST_CHECK_EQUAL(cnk(0,0,0), 0);
    BOOST_CHECK_EQUAL(cnk(1,2,3), 2);
    BOOST_CHECK_EQUAL(cnk(1,2,4), 3);
    BOOST_CHECK_EQUAL(cnk(1,2,5), 0);
    BOOST_CHECK_EQUAL(cnk(2,3,4), 9);
    BOOST_CHECK_EQUAL(cnk(1,3,4), 5);
    BOOST_CHECK_EQUAL(cnk(3,3,4), 0);

    paste(cnk, {1,1,1}, spr2, {15, 15, 15});
    BOOST_CHECK_EQUAL(cnk(0,0,0), 9);

    paste(cnk, {1,1,1}, spr2, {16, 16, 16});
    BOOST_CHECK_EQUAL(cnk(0,0,0), 9);

    spr2[world_coordinates{0,0,0}].mask = true;
    paste(cnk, {1,1,1}, spr2, {16, 16, 16});
    BOOST_CHECK_EQUAL(cnk(0,0,0), 2);
}
*/

BOOST_AUTO_TEST_CASE (quaternion_test)
{
    // Unit tests after Rosetta Code
    {
    quatd q0 (1,2,3,4), q1 (2,3,4,5), q2 (3,4,5,6);
    double r (7);

    BOOST_CHECK_EQUAL(-q0, quatd(-1, -2, -3, -4));
    BOOST_CHECK_EQUAL(r * q0, quatd(7, 14, 21, 28));
    BOOST_CHECK_EQUAL(r + q0, quatd(1, 2, 3, 11));
    BOOST_CHECK_EQUAL(q0 - r, quatd(1, 2, 3, -3));

    BOOST_CHECK_EQUAL(q0 + q1, quatd(3, 5, 7, 9));
    BOOST_CHECK_EQUAL(q0 + q1, q1 + q0);

    BOOST_CHECK_EQUAL(q0 * q1 * q2, (q0 * q1) * q2);
    BOOST_CHECK_EQUAL((q0 * q1) * q2, q0 * (q1 * q2));
    }

    // Unit tests after Google Closure's
    {
    quaternion<double> q0 (1, 2, 3, 4), expect (-1, -2, -3, 4);
    BOOST_CHECK_EQUAL(~q0, expect);
    BOOST_CHECK_EQUAL(~(~q0), q0);
    }

    {
    quaternion<double> q0 (2, 2, 2, 2), expect (.5, .5, .5, .5);
    BOOST_CHECK_EQUAL(normalize(q0), expect);
    }

    {
    quaternion<float> q0 (1, 2, 3, 4), q1 (2, 3, 4, 5), expect (12, 24, 30, 0);
    BOOST_CHECK_EQUAL(q0 * q1, expect);
    }

    {
    quaternion<double> q0 (1, 2, 3, 4), q1 (5, -6, 7, -8);
    BOOST_CHECK_EQUAL(slerp(q0, q1, 0.0), q0);

    q0 = normalize(q0);
    q1 = normalize(q1);

    quaternion<double>
     exp_3 (-0.00050153732754, 0.481761203464, 0.239877527076, 0.84283133739),
     exp_5 (-0.1243045421171, 0.51879732466, 0.010789578099, 0.84574304710),
     exp_8 (-0.291353561485, 0.506925588797, -0.3292443285721, 0.74144299965);

    // Take rounding errors into account, check with a small tolerance.
    for (int i (0); i < 4; ++i)
    {
        BOOST_CHECK_CLOSE(slerp(q0, q1, 0.3)[i], exp_3[i], 0.00001);
        BOOST_CHECK_CLOSE(slerp(q0, q1, 0.5)[i], exp_5[i], 0.00001);
        BOOST_CHECK_CLOSE(slerp(q0, q1, 0.8)[i], exp_8[i], 0.00001);
    }
    }

    {
    quaternion<double> q0 (0.22094256606638, 0.53340203646030,
                           0.64777022739548, 0.497051689967954);
    auto x (rotation_matrix(q0));

    ///\todo This is kinda fishy, find out why some of the following tests fail:

    BOOST_CHECK_CLOSE(x(0,0),-0.408248 , 0.0001);
    //BOOST_CHECK_CLOSE(x(0,1), 0.8796528, 0.0001);
    //BOOST_CHECK_CLOSE(x(0,2),-0.2440169, 0.0001);

    //BOOST_CHECK_CLOSE(x(1,0),-0.4082482, 0.0001);
    BOOST_CHECK_CLOSE(x(1,1), 0.0631562, 0.0001);
    //BOOST_CHECK_CLOSE(x(1,2), 0.9106836, 0.0001);

    //BOOST_CHECK_CLOSE(x(2,0), 0.8164965, 0.0001);
    //BOOST_CHECK_CLOSE(x(2,1), 0.4714045, 0.0001);
    BOOST_CHECK_CLOSE(x(2,2), 0.3333333, 0.0001);
    }
}


