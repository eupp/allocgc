#include <gtest/gtest.h>

#include <unordered_set>

#include "libprecisegc/details/gc_compact.h"
#include "libprecisegc/details/segregated_list.h"

#include "libprecisegc/details/allocators/fixed_size_allocator.h"
#include "libprecisegc/details/allocators/paged_allocator.h"
#include "libprecisegc/details/managed_pool_chunk.h"

#include "rand_util.h"

using namespace precisegc::details;
using namespace precisegc::details::allocators;

namespace {
static const size_t OBJ_SIZE = 32;      // PAGE_SIZE / OBJECTS_PER_PAGE;
static const size_t OBJ_COUNT_1 = 5;    //
static const size_t OBJ_COUNT_2 = 3 * managed_pool_chunk::CHUNK_MAXSIZE;

struct test_type
{
    byte data[OBJ_SIZE];
};

typedef fixed_size_allocator<managed_pool_chunk,
                             paged_allocator,
                             paged_allocator
                            > allocator_t;
}

class gc_compact_test : public ::testing::Test
{
public:
    gc_compact_test()
        : m_chunk(managed_pool_chunk::create(OBJ_SIZE, m_paged_alloc))
    {}

    ~gc_compact_test()
    {
        for (auto ptr: m_allocated) {
            m_alloc.deallocate(managed_cell_ptr(managed_ptr(ptr)), OBJ_SIZE);
        }
        managed_pool_chunk::destroy(m_chunk, OBJ_SIZE, m_paged_alloc);
    }

    paged_allocator m_paged_alloc;
    allocator_t m_alloc;
    managed_pool_chunk m_chunk;
    std::unordered_set<byte*> m_allocated;
};

TEST_F(gc_compact_test, test_two_finger_compact_1)
{
    for (int i = 0; i < OBJ_COUNT_1; ++i) {
        managed_cell_ptr cell_ptr = m_alloc.allocate(OBJ_SIZE);
        m_allocated.insert(cell_ptr.get());
        cell_ptr.set_mark(false);
    }

    auto rng = m_alloc.range();
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

    byte* exp_to = it1->get();
    byte* exp_from = it4->get();

    forwarding_list frwd;
    two_finger_compact(rng, OBJ_SIZE, frwd);

    ASSERT_EQ(1, frwd.size());
    void* from = frwd[0].from();
    void* to = frwd[0].to();

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

    for (size_t i = 0; i < managed_pool_chunk::CHUNK_MINSIZE; ++i) {
        m_chunk.allocate(OBJ_SIZE);
    }

    uniform_rand_generator<size_t> rand_gen(0, managed_pool_chunk::CHUNK_MINSIZE - 1);
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

    forwarding_list frwd;
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
        managed_cell_ptr cell_ptr = m_alloc.allocate(OBJ_SIZE);
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

    auto rng = m_alloc.range();
    forwarding_list frwd;
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

    for (size_t i = 0; i < managed_pool_chunk::CHUNK_MINSIZE; ++i) {
        m_chunk.allocate(OBJ_SIZE);
    }

    uniform_rand_generator<size_t> rand_gen(0, managed_pool_chunk::CHUNK_MINSIZE - 1);
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

    forwarding_list frwd;
    two_finger_compact(rng, OBJ_SIZE, frwd);
    sweep(rng);

    for (auto it = rng.begin(); it != rng.end(); ++it) {
        ASSERT_FALSE(it->get_mark());
    }

    size_t dead_cnt = std::distance(rng.begin(), rng.end()) - LIVE_CNT;
    for (size_t i = 0; i < dead_cnt; ++i) {
        ASSERT_TRUE(m_chunk.memory_available());
        m_chunk.allocate(OBJ_SIZE);
    }
    ASSERT_FALSE(m_chunk.memory_available());
}

TEST_F(gc_compact_test, test_fix_pointers)
{
    segregated_list sl(OBJ_SIZE);
    auto alloc_res = sl.allocate();
    void* ptr = alloc_res.first;
    page_descriptor* pd = alloc_res.second;

    auto offsets = std::vector<size_t>({0});
    typedef class_meta_provider<test_type> provider;
    provider::create_meta(offsets);

    void*& from = * (void**) ptr;
    object_meta* obj_meta = object_meta::get_meta_ptr(ptr, pd->obj_size());
    obj_meta->set_class_meta(provider::get_meta_ptr());
    obj_meta->set_count(1);
    obj_meta->set_object_ptr(ptr);

    forwarding_list forwarding;
    forwarding.emplace_back(from, nullptr, OBJ_SIZE);

    fix_pointers(sl.begin(), sl.end(), OBJ_SIZE, forwarding);

    ASSERT_EQ(nullptr, from);
}