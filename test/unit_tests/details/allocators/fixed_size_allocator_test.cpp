#include <gtest/gtest.h>

#include <iostream>

#include "libprecisegc/details/allocators/fixed_size_allocator.h"
#include "libprecisegc/details/allocators/paged_allocator.h"
#include "libprecisegc/details/allocators/debug_layer.h"
#include "libprecisegc/details/allocators/types.h"

#include "test_chunk.h"

using namespace precisegc::details::allocators;

namespace {
static const size_t OBJ_SIZE = sizeof(size_t);
}

class fixed_size_allocator_test: public ::testing::Test
{
public:
    typedef debug_layer<paged_allocator> allocator_t;
    typedef fixed_size_allocator<test_chunk, allocator_t, allocator_t> fixed_allocator_t;

    fixed_allocator_t m_alloc;
};

TEST_F(fixed_size_allocator_test, test_sizeof)
{
    typedef fixed_size_allocator<test_chunk, paged_allocator, paged_allocator> alloc_t;
    RecordProperty("size", sizeof(alloc_t));
    std::cout << "[          ] sizeof(fixed_size_allocator) = " << sizeof(alloc_t) << std::endl;
}

TEST_F(fixed_size_allocator_test, test_allocate_1)
{
    size_t* ptr = (size_t*) m_alloc.allocate(OBJ_SIZE);
    ASSERT_NE(nullptr, ptr);
    *ptr = 42;

    m_alloc.deallocate((byte*) ptr, OBJ_SIZE);
}

TEST_F(fixed_size_allocator_test, test_allocate_2)
{
    byte* ptr = m_alloc.allocate(OBJ_SIZE);
    m_alloc.deallocate(ptr, OBJ_SIZE);

    ASSERT_EQ(0, m_alloc.get_const_allocator().get_allocated_mem_size());
}

TEST_F(fixed_size_allocator_test, test_allocate_3)
{
    size_t* ptr1 = (size_t*) m_alloc.allocate(OBJ_SIZE);
    size_t* ptr2 = (size_t*) m_alloc.allocate(OBJ_SIZE);

    ASSERT_NE(nullptr, ptr2);
    ASSERT_NE(ptr1, ptr2);
    *ptr2 = 42;

    m_alloc.deallocate((byte*) ptr1, OBJ_SIZE);
    m_alloc.deallocate((byte*) ptr2, OBJ_SIZE);
}