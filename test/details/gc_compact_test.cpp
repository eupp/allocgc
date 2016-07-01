#include <gtest/gtest.h>

#include <iostream>
#include <unordered_set>

#include "libprecisegc/details/gc_compact.h"
#include "libprecisegc/details/forwarding.h"

#include <libprecisegc/details/allocators/list_allocator.hpp>
#include <libprecisegc/details/allocators/page_allocator.hpp>
#include <libprecisegc/details/allocators/managed_pool_chunk.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>

#include "rand_util.h"

using namespace precisegc::details;
using namespace precisegc::details::allocators;

namespace {
static const size_t OBJ_SIZE = 64;      // PAGE_SIZE / OBJECTS_PER_PAGE;
static const size_t OBJ_COUNT_1 = 5;    //
static const size_t OBJ_COUNT_2 = 3 * managed_pool_chunk::CHUNK_MAXSIZE;

struct test_type
{
    byte data[OBJ_SIZE];
};

typedef list_allocator<managed_pool_chunk,
                             page_allocator,
                             default_allocator,
                             utils::dummy_mutex
                            > allocator_t;
}

class gc_compact_test : public ::testing::Test
{
public:
    gc_compact_test()
        : m_chunk(m_paged_alloc.allocate(managed_pool_chunk::CHUNK_MAXSIZE * OBJ_SIZE), managed_pool_chunk::CHUNK_MAXSIZE * OBJ_SIZE, OBJ_SIZE)
    {}

    ~gc_compact_test()
    {
        for (auto ptr: m_allocated) {
            m_alloc.deallocate(managed_ptr(ptr), OBJ_SIZE);
        }
        m_paged_alloc.deallocate(m_chunk.get_mem(), m_chunk.get_mem_size());
    }

    page_allocator m_paged_alloc;
    allocator_t m_alloc;
    managed_pool_chunk m_chunk;
    std::unordered_set<byte*> m_allocated;
};

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
TEST_F(gc_compact_test, test_two_finger_compact_1)
{
    for (int i = 0; i < OBJ_COUNT_1; ++i) {
        managed_ptr cell_ptr = m_alloc.allocate(OBJ_SIZE);
        m_allocated.insert(cell_ptr.get());
        cell_ptr.set_mark(false);
    }

    auto rng = m_alloc.memory_range();
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

    list_forwarding frwd;
    two_finger_compact(rng, OBJ_SIZE, frwd);

    auto frwd_list = frwd.get_list();
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

TEST_F(gc_compact_test, test_two_finger_compact_2)
{
    const size_t LIVE_CNT = 5;
    const size_t CHUNK_SIZE = std::max(4 * LIVE_CNT, (size_t) managed_pool_chunk::CHUNK_MINSIZE);

    for (size_t i = 0; i < CHUNK_SIZE; ++i) {
        m_chunk.allocate(OBJ_SIZE);
    }

    uniform_rand_generator<size_t> rand_gen(0, CHUNK_SIZE - 1);
    auto rng = m_chunk.get_range();
    for (size_t i = 0; i < LIVE_CNT; ++i) {
        size_t rand = rand_gen();
        auto it = std::next(rng.begin(), rand);
        while (it->get_mark()) {
            rand = rand_gen();
            it = std::next(rng.begin(), rand);
        }
        it->set_mark(true);
    }

    list_forwarding frwd;
    two_finger_compact(rng, OBJ_SIZE, frwd);

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

TEST_F(gc_compact_test, test_two_finger_compact_3)
{
    bernoulli_rand_generator mark_gen(0.3);
    bernoulli_rand_generator pin_gen(0.2);
    size_t exp_mark_cnt = 0;
    size_t exp_pin_cnt = 0;
    std::unordered_set<byte*> pinned;
    for (int i = 0; i < OBJ_COUNT_2; ++i) {
        managed_ptr cell_ptr = m_alloc.allocate(OBJ_SIZE);
        m_allocated.insert(cell_ptr.get());
        bool mark = mark_gen();
        bool pin = pin_gen();
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

    auto rng = m_alloc.memory_range();
    list_forwarding frwd;
    two_finger_compact(rng, OBJ_SIZE, frwd);

    size_t mark_cnt = 0;
    size_t pin_cnt = 0;
    for (auto cell_ptr: rng) {
        if (cell_ptr.get_mark()) {
            mark_cnt++;
        }
        if (cell_ptr.get_pin()) {
            pin_cnt++;
            EXPECT_TRUE(pinned.count(cell_ptr.get()));
        }
    }

    EXPECT_EQ(exp_mark_cnt, mark_cnt);
    EXPECT_EQ(exp_pin_cnt, pin_cnt);
}

TEST_F(gc_compact_test, test_compact_and_sweep)
{
    const size_t LIVE_CNT = 5;
    const size_t CHUNK_SIZE = std::max(4 * LIVE_CNT, (size_t) managed_pool_chunk::CHUNK_MINSIZE);

    for (size_t i = 0; i < CHUNK_SIZE; ++i) {
        m_chunk.allocate(OBJ_SIZE);
    }

    uniform_rand_generator<size_t> rand_gen(0, CHUNK_SIZE - 1);
    auto rng = m_chunk.get_range();
    for (size_t i = 0; i < LIVE_CNT; ++i) {
        size_t rand = rand_gen();
        auto it = std::next(rng.begin(), rand);
        while (it->get_mark()) {
            rand = rand_gen();
            it = std::next(rng.begin(), rand);
        }
        it->set_mark(true);
    }

    list_forwarding frwd;
    two_finger_compact(rng, OBJ_SIZE, frwd);

    size_t sweep_cnt = sweep(rng);

    for (auto it = rng.begin(); it != rng.end(); ++it) {
        ASSERT_FALSE(it->get_mark());
    }

    size_t dead_cnt = CHUNK_SIZE - LIVE_CNT;
    size_t free_cnt = std::distance(rng.begin(), rng.end()) - LIVE_CNT;
    ASSERT_EQ(dead_cnt, sweep_cnt);
    for (size_t i = 0; i < free_cnt; ++i) {
        ASSERT_TRUE(m_chunk.memory_available());
        m_chunk.allocate(OBJ_SIZE);
    }
    ASSERT_FALSE(m_chunk.memory_available());
}

TEST_F(gc_compact_test, test_fix_pointers)
{
    managed_ptr cell_ptr = m_chunk.allocate(OBJ_SIZE);
    cell_ptr.set_mark(true);
    byte* ptr = cell_ptr.get();

    test_type val1;
    void* to = &val1;

    test_type val2;
    void*& from = * (void**) ptr;
    from = &val2;

    auto offsets = std::vector<size_t>({0});
    typedef type_meta_provider<test_type> provider;
    provider::create_meta(offsets.begin(), offsets.end());

    object_meta* obj_meta = object_meta::get_meta_ptr(ptr, OBJ_SIZE);
    obj_meta->set_class_meta(provider::get_meta_ptr());
    obj_meta->set_count(1);
    obj_meta->set_object_ptr(ptr);

    list_forwarding forwarding;
    forwarding.create(from, to, OBJ_SIZE);

    auto rng = m_chunk.get_range();
    fix_pointers(rng.begin(), rng.end(), OBJ_SIZE, forwarding);

    ASSERT_EQ(to, from);
}