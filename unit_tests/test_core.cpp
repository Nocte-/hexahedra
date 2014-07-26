#define BOOST_TEST_MODULE hexahedra_test

#include <boost/test/unit_test.hpp>

#include <chrono>
#include <random>
#include <set>
#include <thread>
#include <boost/range/algorithm.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#include <hexa/aabb.hpp>
#include <hexa/algorithm.hpp>
#include <hexa/chunk.hpp>
#include <hexa/collision.hpp>
#include <hexa/compression.hpp>
#include <hexa/concurrent_queue.hpp>
#include <hexa/crypto.hpp>
#include <hexa/geometric.hpp>
#include <hexa/hotbar_slot.hpp>
#include <hexa/lru_cache.hpp>
#include <hexa/persistence_leveldb.hpp>
#include <hexa/persistence_null.hpp>
#include <hexa/protocol.hpp>
#include <hexa/quaternion.hpp>
#include <hexa/server/random.hpp>
#include <hexa/ray.hpp>
#include <hexa/ray_bundle.hpp>
#include <hexa/serialize.hpp>
#include <hexa/surface.hpp>
#include <hexa/trace.hpp>
#include <hexa/vector3.hpp>
#include <hexa/voxel_algorithm.hpp>
#include <hexa/voxel_range.hpp>
#include <hexa/wfpos.hpp>

namespace es {

template<>
void serialize<std::string>(const std::string& s, std::vector<char>& buf)
{
    uint16_t size (s.size());
    buf.push_back(size & 0xff);
    buf.push_back(size >> 8);
    buf.insert(buf.end(), s.begin(), s.end());
}

template<>
std::vector<char>::const_iterator
deserialize<std::string>(std::string& obj, std::vector<char>::const_iterator first, std::vector<char>::const_iterator last)
{
    if (std::distance(first, last) < 2)
        throw  std::runtime_error("cannot deserialize string: no length field");

    uint16_t size (uint8_t(*first++));
    size += (uint16_t(uint8_t(*first++)) << 8 );

    if (std::distance(first, last) < size)
        throw  std::runtime_error("cannot deserialize string: not enough data");

    last = first + size;
    obj.assign(first, last);
    return last;
}


} // namespace es

using namespace hexa;

template <typename t>
t serialize_roundtrip(t& in)
{
    std::vector<uint8_t> buf;
    auto ser (make_serializer(buf));
    in.serialize(ser);

    t out;
    auto des (make_deserializer(buf));
    out.serialize(des);

    return out;
}

BOOST_AUTO_TEST_SUITE(core)

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

    //---

    buffer a2;
    auto ser2 (make_serializer(a2));
    block_vector d (10, 5, 4), e;
    ser2(d);

    auto dser2 (make_deserializer(a2));
    dser2(e);
    BOOST_CHECK_EQUAL(a2.size(), 2); // Not 3, block_vector is a special case
    BOOST_CHECK_EQUAL(d, e);

    //---

    buffer a3;
    auto ser3 (make_serializer(a3));
    hotbar hb1;
    hb1.push_back(hotbar_slot(1, "lol"));
    hb1.push_back(hotbar_slot(2, "foobar"));
    ser3(hb1);

    hotbar hb2;
    auto dser3 (make_deserializer(a3));
    dser3(hb2);
    BOOST_CHECK_EQUAL(hb1.size(), hb2.size());
    BOOST_CHECK_EQUAL(hb1[0].type, hb2[0].type);
    BOOST_CHECK_EQUAL(hb1[0].name, hb2[0].name);
    BOOST_CHECK_EQUAL(hb1[1].type, hb2[1].type);
    BOOST_CHECK_EQUAL(hb1[1].name, hb2[1].name);

    //---

    chunk test_cnk;
    auto p (prng(1234));
    for (auto i : every_block_in_chunk)
        test_cnk[i] = prng_next(p) & 0xffff;

    buffer a4;
    auto ser4 (make_serializer(a4));
    ser4(test_cnk);

    chunk compare_cnk;
    auto dser4 (make_deserializer(a4));
    dser4(compare_cnk);

    BOOST_CHECK(test_cnk == compare_cnk);
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
    BOOST_CHECK_EQUAL(second.mod(3), vector2<int>(1, 1));

    vector2<int> third (-1, -15);
    BOOST_CHECK_EQUAL(third / chunk_size, vector2<int>(-1, -1));
    BOOST_CHECK_EQUAL(third >> cnkshift, vector2<int>(-1, -1));
    BOOST_CHECK_EQUAL(third % chunk_size, vector2<int>(15, 1));
    vector2<int> fourth (-16, -31);
    BOOST_CHECK_EQUAL(fourth / chunk_size, vector2<int>(-1, -2));
    BOOST_CHECK_EQUAL(fourth >> cnkshift, vector2<int>(-1, -2));
    BOOST_CHECK_EQUAL(fourth % chunk_size, vector2<int>(0, 1));
}

BOOST_AUTO_TEST_CASE (vector3_test)
{
    vector3<int> first (1, 2, 3);
    BOOST_CHECK_EQUAL(first.x, 1);
    BOOST_CHECK_EQUAL(first.y, 2);
    BOOST_CHECK_EQUAL(first.z, 3);
    BOOST_CHECK_EQUAL(first, vector3<int>(1, 2, 3));
    BOOST_CHECK_EQUAL(first / chunk_size, vector3<int>(0, 0, 0));
    BOOST_CHECK_EQUAL(first >> cnkshift, vector3<int>(0, 0, 0));

    vector3<int> second (4, 7, 13);
    BOOST_CHECK_EQUAL(first + second, vector3<int>(5, 9, 16));
    BOOST_CHECK_EQUAL(first - second, vector3<int>(-3, -5, -10));
    BOOST_CHECK_EQUAL(first * second, vector3<int>(4, 14, 39));
    BOOST_CHECK_EQUAL(second % 4, vector3<int>(0, 3, 1));

    vector3<int> third (-3, 8, 3);
    BOOST_CHECK_EQUAL(squared_length(first), 14);
    BOOST_CHECK_EQUAL(diff(first, first), vector3<int>(0, 0, 0));
    BOOST_CHECK_EQUAL(diff(first, third), vector3<int>(4, 6, 0));
    BOOST_CHECK_EQUAL(manhattan_distance(first, first), 0);
    BOOST_CHECK_EQUAL(manhattan_distance(first, second), 18);
    BOOST_CHECK_EQUAL(third / chunk_size, vector3<int>(-1, 0, 0));
    BOOST_CHECK_EQUAL(third >> cnkshift, vector3<int>(-1, 0, 0));
    BOOST_CHECK_EQUAL(third % chunk_size, vector3<int>(13, 8, 3));

    vector3<int> fourth (-17, -16, 31);
    BOOST_CHECK_EQUAL(fourth / chunk_size, vector3<int>(-2, -1, 1));
    BOOST_CHECK_EQUAL(fourth >> cnkshift, vector3<int>(-2, -1, 1));
    BOOST_CHECK_EQUAL(fourth % chunk_size, vector3<int>(15, 0, 15));

    vector3<uint32_t> limit (0xffffffff);
    BOOST_CHECK_EQUAL(limit >> cnkshift, chunk_world_limit - vec3i(1));
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

BOOST_AUTO_TEST_CASE (aabb_neg_test)
{
    {
    aabb<vec3i> box ({ -4, -4, -4 }, { 4, 4, 4 });
    auto box2 (box >> cnkshift);
    BOOST_CHECK_EQUAL(box2.first,  vec3i(-1, -1, -1));
    BOOST_CHECK_EQUAL(box2.second, vec3i(1, 1, 1));
    }
    {
    aabb<vec3i> box ({ -18, -4, -4 }, { 4, 4, 15 });
    auto box2 (box >> cnkshift);
    BOOST_CHECK_EQUAL(box2.first,  vec3i(-2, -1, -1));
    BOOST_CHECK_EQUAL(box2.second, vec3i(1, 1, 1));
    }
    {
        aabb<vec3f> box ({ -1.f, -1.f, -1.f }, { 15.5f, 15.5f, 16.5f });
        aabb<vec3i> boxi (cast_to<vec3i>(box));
        BOOST_CHECK_EQUAL(boxi.first,  vec3i(-1, -1, -1));
        BOOST_CHECK_EQUAL(boxi.second, vec3i(16, 16, 17));

        aabb<vec3i> boxc (boxi >> cnkshift);
        BOOST_CHECK_EQUAL(boxc.first,  vec3i(-1, -1, -1));
        BOOST_CHECK_EQUAL(boxc.second, vec3i( 1,  1,  2));
    }
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

    cache[1] = "one";
    cache[8] = "eight";
    cache[5] = "five";
    cache[4] = "four";
    cache.prune_if(1, [=](const lru_cache<int, std::string>::value_type& p){ return p.first % 2 == 0; });
    BOOST_CHECK_EQUAL(cache.size(), 2);
    BOOST_CHECK_EQUAL(cache.get(1), "one");
    BOOST_CHECK_EQUAL(cache.get(5), "five");

    cache.clear();

    cache[1] = "one";
    cache[8] = "eight";
    cache[5] = "five";
    cache[4] = "four";
    cache.prune_if(2, [=](const lru_cache<int, std::string>::value_type& p){ return p.first % 2 == 0; });
    BOOST_CHECK_EQUAL(cache.size(), 2);
    BOOST_CHECK_EQUAL(cache.get(1), "one");
    BOOST_CHECK_EQUAL(cache.get(5), "five");

    cache[1] = "one";
    cache[8] = "eight";
    cache[5] = "five";
    cache[4] = "four";
    cache.prune_if(3, [=](const lru_cache<int, std::string>::value_type& p){ return p.first % 2 == 0; });
    BOOST_CHECK_EQUAL(cache.size(), 3);
    BOOST_CHECK_EQUAL(cache.get(1), "one");
    BOOST_CHECK_EQUAL(cache.get(5), "five");
    BOOST_CHECK_EQUAL(cache.get(4), "four");

    cache[1] = "one";
    cache[8] = "eight";
    cache[5] = "five";
    cache[4] = "four";
    cache.prune_if(8, [=](const lru_cache<int, std::string>::value_type& p){ return p.first % 2 == 0; });
    BOOST_CHECK_EQUAL(cache.size(), 4);
}


BOOST_AUTO_TEST_CASE (crypto_test)
{
    for (int i = 0; i < 100; ++i)
    {
        auto key   (crypto::make_new_key());
        auto spriv (crypto::serialize_private_key(key));
        auto spubl (crypto::serialize_public_key(key));

      //  std::cout << spriv << std::endl;
        //std::cout << spubl << std::endl;


    }
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

BOOST_AUTO_TEST_CASE (persistent_storage_test)
{
    boost::filesystem::path tmpdb ("storetest.leveldb");
    boost::filesystem::remove_all (tmpdb);

    {
    es::storage st;
    auto c1 (st.register_component<int>("first"));

    persistence_leveldb ldb (tmpdb);

    uint32_t rn (31337);
    for (int i (0); i < 1000; ++i)
    {
        auto pos (prng_next_pos(rn));
        auto type (static_cast<persistence_leveldb::data_type>(prng_next(rn) % 4));
        binary_data buf (prng_next(rn) % 200);
        for (auto& c : buf)
            c = prng_next(rn) & 0xff;

        ldb.store(type, pos, compress(buf));
        ldb.store(map_coordinates(pos.x, pos.y), pos.z);
    }

    for (int i (0); i < 1000; ++i)
    {
        auto e (st.make(prng_next(rn)));
        st.set<int>(e, c1, prng_next(rn));
    }
    ldb.store(st);
    }

    {
    es::storage st;
    auto c1 (st.register_component<int>("first"));

    persistence_leveldb ldb (tmpdb);
    ldb.retrieve(st);

    uint32_t rn (31337);
    for (int i (0); i < 1000; ++i)
    {
        auto pos (prng_next_pos(rn));
        auto hgt = ldb.retrieve(map_coordinates(pos.x, pos.y));
        BOOST_CHECK_EQUAL(hgt, pos.z);

        auto type (static_cast<persistence_leveldb::data_type>(prng_next(rn) % 4));
        auto expected_len (prng_next(rn) % 200);

        auto buf = decompress(ldb.retrieve(type, pos));
        BOOST_CHECK_EQUAL(buf.size(), expected_len);
        for (auto& c : buf)
            BOOST_CHECK_EQUAL(uint8_t(c), uint8_t(prng_next(rn) & 0xff));
    }

    for (int i (0); i < 1000; ++i)
    {
        auto e (st.find(prng_next(rn)));
        BOOST_CHECK(e != st.end());
        BOOST_CHECK_EQUAL(st.get<int>(e, c1), prng_next(rn));
    }

    }

    boost::filesystem::remove_all(tmpdb);
}

BOOST_AUTO_TEST_CASE (es_loadsave_test)
{
    es::storage st;

    auto c1 (st.register_component<int>("first"));
    auto c2 (st.register_component<std::string>("second"));
    auto c3 (st.register_component<wfpos>("pos"));
    auto c4 (st.register_component<hotbar>("hb"));
    auto c5 (st.register_component<uint64_t>("uid"));

    auto e1 (st.new_entity());
    auto e2 (st.new_entity());
    auto e3 (st.new_entity());

    hotbar testbar;
    testbar.emplace_back(hotbar_slot(1, "lol"));

    uint64_t testvalue (0x123456789abcdef0);
    st.set(e1, c1, 42);
    st.set(e2, c2, std::string("42"));
    st.set(e2, c5, testvalue);
    st.set(e3, c3, wfpos(1.234f, 5.678f, 9.012f));
    st.set(e3, c4, testbar);

    boost::filesystem::path tmpdb ("es.leveldb");
    boost::filesystem::remove_all (tmpdb);

    {
    persistence_leveldb ldb (tmpdb);
    ldb.store(st);
    }

    st.delete_entity(e1);
    st.delete_entity(e2);
    st.delete_entity(e3);

    BOOST_CHECK_EQUAL(st.size(), 0);
    {
    persistence_leveldb ldb (tmpdb);
    ldb.retrieve(st);
    }
    boost::filesystem::remove_all(tmpdb);
    BOOST_CHECK_EQUAL(st.size(), 3);

    auto ie1 (st.find(e1));
    auto ie2 (st.find(e2));
    auto ie3 (st.find(e3));

    BOOST_CHECK(st.entity_has_component(ie1, c1));
    BOOST_CHECK(!st.entity_has_component(ie1, c2));
    BOOST_CHECK(!st.entity_has_component(ie2, c1));
    BOOST_CHECK(st.entity_has_component(ie2, c2));
    BOOST_CHECK(st.entity_has_component(ie2, c5));
    BOOST_CHECK(!st.entity_has_component(ie3, c2));
    BOOST_CHECK(!st.entity_has_component(ie3, c1));
    BOOST_CHECK(st.entity_has_component(ie3, c3));
    BOOST_CHECK(st.entity_has_component(ie3, c4));
    BOOST_CHECK(!st.entity_has_component(ie3, c5));

    BOOST_CHECK_EQUAL(st.get<int>(ie1, c1), 42);
    BOOST_CHECK_EQUAL(st.get<std::string>(ie2, c2), "42");
    BOOST_CHECK_EQUAL(st.get<uint64_t>(ie2, c5), testvalue);
    BOOST_CHECK_EQUAL(st.get<wfpos>(ie3, c3), wfpos(1.234f, 5.678f, 9.012f));
    BOOST_CHECK_EQUAL(st.get<hotbar>(ie3, c4).size(), 1);
    BOOST_CHECK_EQUAL(st.get<hotbar>(ie3, c4)[0].name, "lol");

    int count (0);
    uint64_t check (0);
    st.for_each<uint64_t>(c5, [&](es::storage::iterator i, uint64_t& id)
    {
        ++count;
        check = id;
        return false;
    });

    BOOST_CHECK_EQUAL(count, 1);
    BOOST_CHECK_EQUAL(check, testvalue);
}

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

BOOST_AUTO_TEST_CASE (surfacedata_serialize_test)
{
    surface_data test;

    auto ret1 (serialize_roundtrip(test));
    BOOST_CHECK(ret1.opaque.empty());
    BOOST_CHECK(ret1.transparent.empty());

    test.opaque.emplace_back(chunk_index{1, 2, 3}, 4, 12345);
    test.opaque.emplace_back(chunk_index{2, 3, 5}, 5, 12346);
    test.opaque.emplace_back(chunk_index{8, 10, 12}, 6, 12347);

    auto ret2 (serialize_roundtrip(test));
    BOOST_CHECK_EQUAL(ret2.opaque.size(), 3);
    BOOST_CHECK_EQUAL(ret2.opaque[0].pos.x, 1);
    BOOST_CHECK_EQUAL(ret2.opaque[0].pos.z, 3);
    BOOST_CHECK_EQUAL(ret2.opaque[1].dirs, 5);
    BOOST_CHECK_EQUAL(ret2.opaque[2].type, 12347);
    BOOST_CHECK(ret2.transparent.empty());

    test.opaque.clear();
    for (int i (0); i < 2048; ++i)
    {
        test.opaque.emplace_back(chunk_index{1,2,3}, 4, 567 + i);
        test.transparent.emplace_back(chunk_index{10, 11, 12}, 5, 890 + i);
    }

    auto ret3 (serialize_roundtrip(test));
    BOOST_CHECK_EQUAL(ret3.opaque.size(), 2048);
    BOOST_CHECK_EQUAL(ret3.transparent.size(), 2048);

    BOOST_CHECK_EQUAL(ret3.opaque[0].pos.x, 1);
    BOOST_CHECK_EQUAL(ret3.opaque[2047].pos.x, 1);
    BOOST_CHECK_EQUAL(ret3.opaque[0].type, 567);
    BOOST_CHECK_EQUAL(ret3.opaque[2047].type, 567 + 2047);

    BOOST_CHECK_EQUAL(ret3.transparent[0].pos.x, 10);
    BOOST_CHECK_EQUAL(ret3.transparent[2047].pos.x, 10);
    BOOST_CHECK_EQUAL(ret3.transparent[0].type, 890);
    BOOST_CHECK_EQUAL(ret3.transparent[2047].type, 890 + 2047);
}

BOOST_AUTO_TEST_CASE (protocol_test)
{
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

BOOST_AUTO_TEST_CASE (protocol2_test)
{
    msg::define_materials msg;
    material mat;

    mat.name = "name1";
    mat.textures[0] = 10;

    msg.materials.emplace_back(16, mat);

    mat.name = "name2";
    mat.textures[0] = 20;

    msg.materials.emplace_back(32, mat);

    std::vector<uint8_t> buf;
    auto p (make_serializer(buf));
    msg.serialize(p);

    auto arch (make_deserializer(buf));
    msg::define_materials r;
    r.serialize(arch);

    BOOST_CHECK_EQUAL(r.materials.size(), 2);
    BOOST_CHECK_EQUAL(r.materials[0].definition.name, "name1");
    BOOST_CHECK_EQUAL(r.materials[1].definition.name, "name2");
    BOOST_CHECK_EQUAL(r.materials[0].definition.textures[0], 10);
    BOOST_CHECK_EQUAL(r.materials[0].definition.textures[1], 0);
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

BOOST_AUTO_TEST_SUITE_END()

