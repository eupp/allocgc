#include <gtest/gtest.h>

#include <unordered_set>

#include <libprecisegc/details/allocators/mso_allocator.hpp>
#include <libprecisegc/details/compacting/two_finger_compactor.hpp>
#include <libprecisegc/details/allocators/core_allocator.hpp>
#include <libprecisegc/details/types.hpp>

#include "rand_util.h"
#include "test_forwarding.hpp"

using namespace precisegc::details;
using namespace precisegc::details::allocators;
using namespace precisegc::details::compacting;

namespace {
static const size_t OBJ_SIZE = 64;      // PAGE_SIZE / OBJECTS_PER_PAGE;

struct test_type
{
    byte data[OBJ_SIZE];
};

typedef mso_allocator allocator_t;

}

template <typename Compactor>
struct compactor_test : public ::testing::Test
{
    allocator_t alloc;
    Compactor compactor;
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
TYPED_TEST(compactor_test, test_compact_1)
{
    static const size_t OBJ_COUNT = 5;
    for (int i = 0; i < OBJ_COUNT; ++i) {
        gc_alloc_descriptor cell_ptr = this->alloc.allocate(OBJ_SIZE);
        cell_ptr.set_mark(false);
        cell_ptr.set_pin(false);
    }

    auto rng = this->alloc.memory_range();
    auto it0 = std::next(rng.begin(), 0);
    auto it1 = std::next(rng.begin(), 1);
    auto it2 = std::next(rng.begin(), 2);
    auto it3 = std::next(rng.begin(), 3);
    auto it4 = std::next(rng.begin(), 4);

    // mark & pin some objects
    it0->set_mark(true);
    it3->set_mark(true);
    it3->set_pin(true);
    it4->set_mark(true);

//    for (auto p: rng) {
//        std::cout << (void*) p.get() << " " << p.get_mark() << " " << p.get_pin() << std::endl;
//    }
//    std::cout << std::distance(rng.begin(), rng.end()) << std::endl;

    byte* exp_to = it1->get();
    byte* exp_from = it4->get();

    test_forwarding frwd;
    size_t copied = this->compactor(rng, frwd);

    ASSERT_EQ(OBJ_SIZE, copied);

    auto frwd_list = frwd.get_forwarding_list();
    ASSERT_EQ(1, frwd_list.size());
    void* from = frwd_list[0].from;
    void* to = frwd_list[0].to;

    ASSERT_EQ(exp_from, from);
    ASSERT_EQ(exp_to, to);

    ASSERT_TRUE(it0->get_mark());
    ASSERT_TRUE(it1->get_mark());
    ASSERT_TRUE(it3->get_mark());
    ASSERT_TRUE(it3->get_pin());

    ASSERT_FALSE(it2->get_mark());
    ASSERT_FALSE(it4->get_mark());
}

TYPED_TEST(compactor_test, test_compact_2)
{
    const size_t LIVE_CNT = 5;
    const size_t ALLOC_CNT = 4 * LIVE_CNT;

    for (size_t i = 0; i < ALLOC_CNT; ++i) {
        gc_alloc_descriptor cell_ptr = this->alloc.allocate(OBJ_SIZE);
        cell_ptr.set_mark(false);
        cell_ptr.set_pin(false);
    }

    uniform_rand_generator<size_t> rand_gen(0, ALLOC_CNT - 1);
    auto rng = this->alloc.memory_range();
    for (size_t i = 0; i < LIVE_CNT; ++i) {
        size_t rand = rand_gen();
        auto it = std::next(rng.begin(), rand);
        while (it->get_mark()) {
            rand = rand_gen();
            it = std::next(rng.begin(), rand);
        }
        it->set_mark(true);
    }

    test_forwarding frwd;
    size_t copied = this->compactor(rng, frwd);

    ASSERT_LE(copied, OBJ_SIZE * LIVE_CNT);

    auto live_begin = rng.begin();
    auto live_end = std::next(live_begin, LIVE_CNT);
    auto dead_begin = live_end;
    auto dead_end = rng.end();
    size_t dead_cnt = std::distance(dead_begin, dead_end);
    for (auto it = live_begin; it != live_end; ++it) {
        ASSERT_TRUE(it->get_mark());
    }
    for (auto it = dead_begin; it != dead_end; ++it) {
        ASSERT_FALSE(it->get_mark());
    }
}

TYPED_TEST(compactor_test, test_compact_3)
{
    static const size_t OBJ_COUNT = 512;
    bernoulli_rand_generator mark_gen(0.3);
    bernoulli_rand_generator pin_gen(0.2);
    size_t exp_mark_cnt = 0;
    size_t exp_pin_cnt = 0;
    std::unordered_set<byte*> pinned;
    for (int i = 0; i < OBJ_COUNT; ++i) {
        gc_alloc_descriptor cell_ptr = this->alloc.allocate(OBJ_SIZE);
        bool pin = pin_gen();
        bool mark = mark_gen() || pin;
        cell_ptr.set_mark(mark);
        cell_ptr.set_pin(pin);
        if (mark) {
            exp_mark_cnt++;
        }
        if (pin) {
            exp_pin_cnt++;
            pinned.insert(cell_ptr.get());
        }
    }

    auto rng = this->alloc.memory_range();
    test_forwarding frwd;
    size_t copied = this->compactor(rng, frwd);

    EXPECT_LE(copied, exp_mark_cnt * OBJ_SIZE);

    size_t pin_cnt = 0;
    size_t mark_cnt = 0;
    for (auto cell_ptr: rng) {
        if (cell_ptr.get_mark()) {
            mark_cnt++;
        }
        if (cell_ptr.get_pin()) {
            pin_cnt++;
            EXPECT_TRUE(pinned.count(cell_ptr.get()));
        }
    }

    EXPECT_EQ(exp_pin_cnt, pin_cnt);
    EXPECT_EQ(exp_mark_cnt, mark_cnt);
}