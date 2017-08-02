#include <gtest/gtest.h>

#include <unordered_set>

#include <liballocgc/details/allocators/gc_pool_allocator.hpp>
#include <liballocgc/details/compacting/two_finger_compactor.hpp>
#include <liballocgc/details/allocators/gc_core_allocator.hpp>
#include <liballocgc/gc_type_meta.hpp>
#include <liballocgc/gc_common.hpp>

#include "utils.hpp"
#include "rand_util.h"
#include "test_forwarding.hpp"

using namespace allocgc;
using namespace allocgc::details;
using namespace allocgc::details::allocators;
using namespace allocgc::details::compacting;

namespace {
static const size_t OBJ_SIZE = 16;      // PAGE_SIZE / OBJECTS_PER_PAGE;
static const size_t ALLOC_SIZE = 64;

struct test_type
{
    byte data[OBJ_SIZE];
};

const gc_type_meta* type_meta = gc_type_meta_factory<test_type>::create();

typedef gc_pool_allocator allocator_t;

}

template <typename Compactor>
struct compactor_test : public ::testing::Test
{
    compactor_test()
        : rqst(OBJ_SIZE, 1, type_meta, &buf)
    {
        alloc.init(&bucket_policy, &core_alloc);
    }

    gc_bucket_policy bucket_policy;
    gc_core_allocator core_alloc;
    gc_pool_allocator alloc;
    Compactor compactor;
    gc_buf buf;
    gc_alloc::request rqst;
};

typedef ::testing::Types<two_finger_compactor> test_compactor_types;
TYPED_TEST_CASE(compactor_test, test_compactor_types);

/**
 * In this test a small pool is compated by the two-finger-compact procedure and resulting forwarding is checked.
 * Pool layout illustrated below ( x - marked (occupied) cell, # - pinned cell).
 *
 *      *********************
 *      * 0 | 1 | 2 | 3 | 4 *
 *      *********************
 *      * x |   |   | # | x *
 *      *********************
 *
 * After compacting pool should look like:
 *
 *      *********************
 *      * 0 | 1 | 2 | 3 | 4 *
 *      *********************
 *      * x | x |   | # |   *
 *      *********************
 */
//TYPED_TEST(compactor_test, test_compact_1)
//{
//    static const size_t OBJ_COUNT = 5;
//    gc_alloc::response rsps[OBJ_COUNT];
//    for (int i = 0; i < OBJ_COUNT; ++i) {
//        rsps[i] = this->alloc.allocate(this->rqst, ALLOC_SIZE);
//    }
//
//    // mark & pin some objects
//    commit(rsps[0]);
//    set_mark(rsps[0], true);
//    commit(rsps[3]);
//    set_mark(rsps[3], true);
//    set_pin(rsps[3], true);
//    commit(rsps[4]);
//    commit(rsps[4]);
//    set_mark(rsps[4], true);
//
////    for (auto p: rng) {
////        std::cout << (void*) p.get() << " " << p.get_mark() << " " << p.get_pin() << std::endl;
////    }
////    std::cout << std::distance(rng.begin(), rng.end()) << std::endl;
//
//    byte* exp_to = rsps[1].cell_start();
//    byte* exp_from = rsps[4].cell_start();
//
//    gc_collect_stat stat;
//    test_forwarding frwd;
//    auto rng = this->alloc.memory_range();
//    this->compactor(rng, frwd, stat);
//
////    ASSERT_EQ(ALLOC_SIZE, stat.mem_freed);
//    ASSERT_EQ(ALLOC_SIZE, stat.mem_moved);
//
//    auto frwd_list = frwd.get_forwarding_list();
//    ASSERT_EQ(1, frwd_list.size());
//    void* from = frwd_list[0].from;
//    void* to = frwd_list[0].to;
//
//    ASSERT_EQ(exp_from, from);
//    ASSERT_EQ(exp_to, to);
//
//    ASSERT_TRUE(get_mark(rsps[0]));
//    ASSERT_TRUE(get_mark(rsps[1]));
//    ASSERT_TRUE(get_mark(rsps[3]));
//    ASSERT_TRUE(get_pin(rsps[3]));
//
//    ASSERT_EQ(gc_lifetime_tag::LIVE, get_lifetime_tag(rsps[0]));
//    ASSERT_EQ(gc_lifetime_tag::LIVE, get_lifetime_tag(rsps[1]));
//    ASSERT_EQ(gc_lifetime_tag::LIVE, get_lifetime_tag(rsps[3]));
//
//    ASSERT_FALSE(get_mark(rsps[2]));
//    ASSERT_FALSE(get_mark(rsps[4]));
//
//    ASSERT_EQ(gc_lifetime_tag::FREE, get_lifetime_tag(rsps[2]));
//    ASSERT_EQ(gc_lifetime_tag::FREE, get_lifetime_tag(rsps[4]));
//}

//TYPED_TEST(compactor_test, test_compact_2)
//{
//    const size_t LIVE_CNT = 5;
//    const size_t ALLOC_CNT = 4 * LIVE_CNT;
//    gc_alloc::response rsps[ALLOC_CNT];
//
//    for (size_t i = 0; i < ALLOC_CNT; ++i) {
//        rsps[i] = this->alloc.allocate(this->rqst, ALLOC_SIZE);
//    }
//
//    uniform_rand_generator<size_t> rand_gen(0, ALLOC_CNT - 1);
//    auto rng = this->alloc.memory_range();
//    for (size_t i = 0; i < LIVE_CNT; ++i) {
//        size_t rand = rand_gen();
//        gc_alloc::response* rsp = rsps + rand;
//        while (get_mark(*rsp)) {
//            rand = rand_gen();
//            rsp = rsps + rand;
//        }
//        commit(*rsp);
//        set_mark(*rsp, true);
//    }
//
//    gc_collect_stat stat;
//    test_forwarding frwd;
//    this->compactor(rng, frwd, stat);
//
//    ASSERT_GE(ALLOC_SIZE * LIVE_CNT, stat.mem_moved);
//
//    auto live_begin = rng.begin();
//    auto live_end = std::next(live_begin, LIVE_CNT);
//    auto dead_begin = live_end;
//    auto dead_end = rng.end();
//    size_t dead_cnt = std::distance(dead_begin, dead_end);
//    for (auto it = live_begin; it != live_end; ++it) {
//        ASSERT_TRUE(it->get_mark());
//        ASSERT_TRUE(it->get_lifetime_tag() == gc_lifetime_tag::LIVE);
//    }
//    for (auto it = dead_begin; it != dead_end; ++it) {
//        ASSERT_FALSE(it->get_mark());
//        ASSERT_TRUE(it->get_lifetime_tag() == gc_lifetime_tag::FREE ||
//                    it->get_lifetime_tag() == gc_lifetime_tag::GARBAGE);
//    }
//}

//TYPED_TEST(compactor_test, test_compact_3)
//{
//    static const size_t OBJ_COUNT = 512;
//    bernoulli_rand_generator mark_gen(0.3);
//    bernoulli_rand_generator pin_gen(0.2);
//    size_t exp_mark_cnt = 0;
//    size_t exp_pin_cnt = 0;
//    std::unordered_set<byte*> pinned;
//    for (int i = 0; i < OBJ_COUNT; ++i) {
//        gc_alloc::response rsp = this->alloc.allocate(this->rqst, ALLOC_SIZE);
//        bool pin = pin_gen();
//        bool mark = mark_gen() || pin;
//        if (mark) {
//            exp_mark_cnt++;
//            commit(rsp);
//            set_mark(rsp, true);
//        }
//        if (pin) {
//            exp_pin_cnt++;
//            set_pin(rsp, true);
//            pinned.insert(rsp.cell_start());
//        }
//    }
//
//    auto rng = this->alloc.memory_range();
//
//    gc_collect_stat stat;
//    test_forwarding frwd;
//    this->compactor(rng, frwd, stat);
//
//    EXPECT_GE(exp_mark_cnt * ALLOC_SIZE, stat.mem_moved);
//
//    size_t pin_cnt = 0;
//    size_t mark_cnt = 0;
//    for (auto cell_ptr: rng) {
//        if (cell_ptr.get_mark()) {
//            mark_cnt++;
//        }
//        if (cell_ptr.get_pin()) {
//            pin_cnt++;
//            EXPECT_TRUE(pinned.count(cell_ptr.cell_start()));
//        }
//    }
//
//    EXPECT_EQ(exp_pin_cnt, pin_cnt);
//    EXPECT_EQ(exp_mark_cnt, mark_cnt);
//}