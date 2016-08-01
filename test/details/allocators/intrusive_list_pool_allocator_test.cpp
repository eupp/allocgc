#include <gtest/gtest.h>

#include <iostream>
#include <iterator>

#include <libprecisegc/details/allocators/intrusive_list_pool_allocator.hpp>
#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/debug_layer.hpp>
#include <libprecisegc/details/types.hpp>

#include "test_chunk.h"

using namespace precisegc::details;
using namespace precisegc::details::allocators;

namespace {
static const size_t OBJ_SIZE = 8;
}

class intrusive_list_pool_allocator_test: public ::testing::Test
{
public:
    typedef debug_layer<default_allocator> allocator_t;
    typedef intrusive_list_pool_allocator<test_chunk, allocator_t> intrusive_list_pool_allocator_t;

    intrusive_list_pool_allocator_t m_alloc;
};

TEST_F(intrusive_list_pool_allocator_test, test_allocate_1)
{
    size_t* ptr = (size_t*) m_alloc.allocate(OBJ_SIZE);
    ASSERT_NE(nullptr, ptr);
    *ptr = 42;
}

TEST_F(intrusive_list_pool_allocator_test, test_allocate_2)
{
    byte* ptr = m_alloc.allocate(OBJ_SIZE);
    m_alloc.deallocate(ptr, OBJ_SIZE);
    m_alloc.shrink(OBJ_SIZE);

    ASSERT_EQ(0, m_alloc.upstream_allocator().get_allocated_mem_size());
}

TEST_F(intrusive_list_pool_allocator_test, test_allocate_3)
{
    size_t* ptr1 = (size_t*) m_alloc.allocate(OBJ_SIZE);
    size_t* ptr2 = (size_t*) m_alloc.allocate(OBJ_SIZE);

    ASSERT_NE(nullptr, ptr2);
    ASSERT_NE(ptr1, ptr2);
    *ptr2 = 42;
}

