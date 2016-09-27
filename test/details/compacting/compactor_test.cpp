#include <gtest/gtest.h>

#include <libprecisegc/details/compacting/two_finger_compactor.hpp>
#include <libprecisegc/details/allocators/list_allocator.hpp>
#include <libprecisegc/details/allocators/core_allocator.hpp>
#include <libprecisegc/details/allocators/managed_pool_chunk.hpp>
#include <libprecisegc/details/allocators/cache_policies.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>
#include <libprecisegc/details/types.hpp>

#include "test_forwarding.hpp"

using namespace precisegc::details;
using namespace precisegc::details::allocators;
using namespace precisegc::details::compacting;

namespace {
static const size_t OBJ_SIZE = 64;      // PAGE_SIZE / OBJECTS_PER_PAGE;
static const size_t CHUNK_SIZE = managed_pool_chunk::chunk_size(OBJ_SIZE);

struct test_type
{
    byte data[OBJ_SIZE];
};

}

template <typename Compactor>
struct compactor_test : public ::testing::Test
{
    compactor_test()
        : chunk(core_allocator::allocate(CHUNK_SIZE), CHUNK_SIZE, OBJ_SIZE)
    {}

    ~compactor_test()
    {
        core_allocator::deallocate(chunk.get_mem(), chunk.get_mem_size());
    }

    managed_pool_chunk chunk;
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
        managed_ptr cell_ptr = this->chunk.allocate(OBJ_SIZE);
        cell_ptr.set_mark(false);
        cell_ptr.set_pin(false);
    }

    auto rng = this->chunk.memory_range();
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