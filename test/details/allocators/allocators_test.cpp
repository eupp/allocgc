#include <gtest/gtest.h>

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/allocators/list_allocator.hpp>
#include <libprecisegc/details/allocators/debug_layer.hpp>
#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/intrusive_list_allocator.hpp>
#include <libprecisegc/details/allocators/pool_allocator.hpp>
#include <libprecisegc/details/allocators/cache_policies.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>

#include "test_chunk.h"

using namespace precisegc::details;
using namespace precisegc::details::allocators;

template <typename Alloc>
struct list_allocator_test : public ::testing::Test
{
    Alloc alloc;
};

namespace {
static const size_t OBJ_SIZE = sizeof(size_t);

typedef debug_layer<default_allocator> debug_allocator_t;

typedef intrusive_list_allocator<
          test_chunk
        , debug_allocator_t
        , always_expand
        , utils::dummy_mutex
    > intrusive_list_allocator_t;

typedef list_allocator<
          debug_allocator_t
    > list_allocator_t;
}

typedef pool_allocator<
        debug_allocator_t
    > pool_allocator_t;

TEST(list_allocator_test, test_allocate_1)
{
    list_allocator_t alloc;

    byte* ptr = alloc.allocate(OBJ_SIZE);
    ASSERT_NE(nullptr, ptr);
    *ptr = 42;

    alloc.deallocate(ptr, OBJ_SIZE);
}

TEST(list_allocator_test, test_allocate_2)
{
    list_allocator_t alloc;

    byte* ptr1 = alloc.allocate(OBJ_SIZE);
    byte* ptr2 = alloc.allocate(OBJ_SIZE);

    ASSERT_NE(nullptr, ptr2);
    ASSERT_NE(ptr1, ptr2);
    *ptr2 = 42;

    alloc.deallocate(ptr1, OBJ_SIZE);
    alloc.deallocate(ptr2, OBJ_SIZE);
}

TEST(list_allocator_test, test_deallocate)
{
    list_allocator_t alloc;

    byte* ptr = alloc.allocate(OBJ_SIZE);
    alloc.deallocate(ptr, OBJ_SIZE);

    ASSERT_EQ(0, alloc.upstream_allocator().get_allocated_mem_size());
}

TEST(list_allocator_test, test_range)
{
    list_allocator_t alloc;

    byte* ptr1 = alloc.allocate(OBJ_SIZE);
    byte* ptr2 = alloc.allocate(OBJ_SIZE);

    auto rng   = alloc.memory_range();
    auto first = rng.begin();
    auto last  = rng.end();

    ASSERT_EQ(2, std::distance(first, last));
    ASSERT_EQ(ptr1, *first);
    ASSERT_EQ(ptr2, *(++first));

    alloc.deallocate(ptr1, OBJ_SIZE);
    alloc.deallocate(ptr2, OBJ_SIZE);
}

TEST(pool_allocator_test, test_allocate_1)
{
    pool_allocator_t alloc;

    byte* ptr = alloc.allocate(OBJ_SIZE);
    ASSERT_NE(nullptr, ptr);
    *ptr = 42;
}

TEST(pool_allocator_test, test_allocate_2)
{
    pool_allocator_t alloc;

    byte* ptr1 = alloc.allocate(OBJ_SIZE);
    byte* ptr2 = alloc.allocate(OBJ_SIZE);

    ASSERT_NE(nullptr, ptr2);
    ASSERT_NE(ptr1, ptr2);
    *ptr2 = 42;
}

TEST(pool_allocator_test, test_shrink)
{
    pool_allocator_t alloc;

    for (size_t i = 0; i < 3; ++i) {
        byte* ptr = alloc.allocate(OBJ_SIZE);
        alloc.deallocate(ptr, OBJ_SIZE);
    }

    alloc.shrink(OBJ_SIZE);

    ASSERT_EQ(0, alloc.upstream_allocator().get_allocated_mem_size());
}

//typedef ::testing::Types<list_allocator_t, intrusive_list_allocator_t, pool_allocator_t> test_list_alloc_types;
//TYPED_TEST_CASE(list_allocator_test, test_list_alloc_types);
//
//TYPED_TEST(list_allocator_test, test_allocate_1)
//{
//    byte* ptr = this->alloc.allocate(OBJ_SIZE);
//    ASSERT_NE(nullptr, ptr);
//    *ptr = 42;
//
//    this->alloc.deallocate(ptr, OBJ_SIZE);
//}
//
//TYPED_TEST(list_allocator_test, test_allocate_2)
//{
//    byte* ptr1 = this->alloc.allocate(OBJ_SIZE);
//    byte* ptr2 = this->alloc.allocate(OBJ_SIZE);
//
//    ASSERT_NE(nullptr, ptr2);
//    ASSERT_NE(ptr1, ptr2);
//    *ptr2 = 42;
//
//    this->alloc.deallocate(ptr1, OBJ_SIZE);
//    this->alloc.deallocate(ptr2, OBJ_SIZE);
//}
//
//TYPED_TEST(list_allocator_test, test_deallocate)
//{
//    byte* ptr = this->alloc.allocate(OBJ_SIZE);
//    this->alloc.deallocate(ptr, OBJ_SIZE);
//
//    ASSERT_EQ(0, this->alloc.upstream_allocator().get_allocated_mem_size());
//}
//
//TYPED_TEST(list_allocator_test, test_shrink)
//{
//    for (size_t i = 0; i < 3; ++i) {
//        this->alloc.allocate(OBJ_SIZE);
//    }
//
//    this->alloc.apply_to_chunks([] (test_chunk& chk) {
//        chk.set_empty();
//    });
//
//    this->alloc.shrink();
//
//    ASSERT_EQ(0, this->alloc.upstream_allocator().get_allocated_mem_size());
//}
//
//TYPED_TEST(list_allocator_test, test_range)
//{
//    byte* ptr1 = this->alloc.allocate(OBJ_SIZE);
//    byte* ptr2 = this->alloc.allocate(OBJ_SIZE);
//
//    auto rng   = this->alloc.memory_range();
//    auto first = rng.begin();
//    auto last  = rng.end();
//
//    ASSERT_EQ(2, std::distance(first, last));
//    ASSERT_EQ(ptr1, *first);
//    ASSERT_EQ(ptr2, *(++first));
//
//    this->alloc.deallocate(ptr1, OBJ_SIZE);
//    this->alloc.deallocate(ptr2, OBJ_SIZE);
//}