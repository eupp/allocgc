#include <gtest/gtest.h>

#include <libprecisegc/details/allocators/debug_layer.hpp>
#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/intrusive_list_allocator.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>

#include "test_descriptor.hpp"

using namespace precisegc::details;
using namespace precisegc::details::allocators;

namespace {
static const size_t OBJ_SIZE = 8;
}

class intrusive_list_allocator_test : public ::testing::Test
{
public:
    typedef debug_layer<default_allocator> allocator_t;
    typedef intrusive_list_allocator<test_descriptor, allocator_t, utils::dummy_mutex> freelist_allocator_t;

    freelist_allocator_t alloc;
};

TEST_F(intrusive_list_allocator_test, test_allocate)
{
    size_t* ptr = (size_t*) alloc.allocate(OBJ_SIZE);
    ASSERT_NE(nullptr, ptr);
    *ptr = 42;
}

TEST_F(intrusive_list_allocator_test, test_deallocate)
{
    byte* ptr = alloc.allocate(OBJ_SIZE);
    alloc.deallocate(ptr, OBJ_SIZE);

    ASSERT_EQ(0, alloc.upstream_allocator().get_allocated_mem_size());
}

TEST_F(intrusive_list_allocator_test, test_shrink)
{
    for (size_t i = 0; i < 3; ++i) {
        alloc.allocate(OBJ_SIZE);
    }

    alloc.apply_to_descriptors([] (test_descriptor& descriptor) {
        descriptor.set_empty(true);
    });

    alloc.shrink();

    ASSERT_EQ(0, alloc.upstream_allocator().get_allocated_mem_size());
}

TEST_F(intrusive_list_allocator_test, test_range)
{
    byte* ptr1 = alloc.allocate(OBJ_SIZE);
    byte* ptr2 = alloc.allocate(OBJ_SIZE);

    auto rng   = alloc.memory_range();
    auto first = rng.begin();
    auto last  = rng.end();

    ASSERT_EQ(2, std::distance(first, last));
    ASSERT_EQ(ptr2, *first);
    ASSERT_EQ(ptr1, *(++first));
}