#include <gtest/gtest.h>

#include <libprecisegc/details/allocators/fixsize_freelist_allocator.hpp>
#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/debug_layer.hpp>

using namespace precisegc::details;
using namespace precisegc::details::allocators;

namespace {
static const size_t OBJ_SIZE = sizeof(size_t);
}

struct fixsize_freelist_allocator_test : public ::testing::Test
{
    typedef fixsize_freelist_allocator<debug_layer<default_allocator>> alloc_t;

    alloc_t alloc;
};


TEST_F(fixsize_freelist_allocator_test, test_allocate_1)
{
    size_t* ptr = (size_t*) alloc.allocate(OBJ_SIZE);
    ASSERT_NE(nullptr, ptr);
    *ptr = 42;

    ASSERT_EQ(OBJ_SIZE, alloc.upstream_allocator().get_allocated_mem_size());
}

TEST_F(fixsize_freelist_allocator_test, test_allocate_2)
{
    byte* ptr1 = alloc.allocate(OBJ_SIZE);
    alloc.deallocate(ptr1, OBJ_SIZE);
    ASSERT_EQ(OBJ_SIZE, alloc.upstream_allocator().get_allocated_mem_size());

    byte* ptr2 = alloc.allocate(OBJ_SIZE);
    ASSERT_EQ(ptr1, ptr2);
    ASSERT_EQ(OBJ_SIZE, alloc.upstream_allocator().get_allocated_mem_size());
}

TEST_F(fixsize_freelist_allocator_test, test_shrink)
{
    byte* ptr = alloc.allocate(OBJ_SIZE);
    alloc.deallocate(ptr, OBJ_SIZE);
    alloc.shrink(OBJ_SIZE);
    ASSERT_EQ(0, alloc.upstream_allocator().get_allocated_mem_size());
}