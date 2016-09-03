#include <gtest/gtest.h>

#include <iostream>
#include <iterator>

#include <libprecisegc/details/allocators/list_allocator.hpp>
#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/debug_layer.hpp>
#include <libprecisegc/details/allocators/cache_policies.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>
#include <libprecisegc/details/types.hpp>

#include "test_chunk.h"

using namespace precisegc::details;
using namespace precisegc::details::allocators;

namespace {
static const size_t OBJ_SIZE = 8;
}

class list_allocator_test: public ::testing::Test
{
public:
    typedef debug_layer<default_allocator> allocator_t;
    typedef list_allocator<
            test_chunk,
            allocator_t,
            allocator_t,
            single_chunk_cache,
            utils::dummy_mutex> list_allocator_t;

    list_allocator_t alloc;
};

TEST_F(list_allocator_test, test_sizeof)
{
    typedef list_allocator<
            test_chunk,
            default_allocator,
            default_allocator,
            always_expand,
            utils::dummy_mutex> alloc_t;
    RecordProperty("size", sizeof(alloc_t));
    std::cout << "[          ] sizeof(list_allocator) = " << sizeof(alloc_t) << std::endl;
}

TEST_F(list_allocator_test, test_allocate_1)
{
    size_t* ptr = (size_t*) alloc.allocate(OBJ_SIZE);
    ASSERT_NE(nullptr, ptr);
    *ptr = 42;
}

TEST_F(list_allocator_test, test_allocate_2)
{
    byte* ptr = alloc.allocate(OBJ_SIZE);
    alloc.deallocate(ptr, OBJ_SIZE);

    ASSERT_EQ(0, alloc.upstream_allocator().get_allocated_mem_size());
}

TEST_F(list_allocator_test, test_allocate_3)
{
    size_t* ptr1 = (size_t*) alloc.allocate(OBJ_SIZE);
    size_t* ptr2 = (size_t*) alloc.allocate(OBJ_SIZE);

    ASSERT_NE(nullptr, ptr2);
    ASSERT_NE(ptr1, ptr2);
    *ptr2 = 42;
}

TEST_F(list_allocator_test, test_range)
{
    byte* ptr1 = alloc.allocate(OBJ_SIZE);
    byte* ptr2 = alloc.allocate(OBJ_SIZE);

    auto rng   = alloc.memory_range();
    auto first = rng.begin();
    auto last  = rng.end();

    ASSERT_EQ(2, std::distance(first, last));
    ASSERT_EQ(ptr1, *first);
    ASSERT_EQ(ptr2, *(++first));
}