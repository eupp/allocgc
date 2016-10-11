#include <gtest/gtest.h>

#include <libprecisegc/details/allocators/mso_allocator.hpp>
#include <libprecisegc/details/compacting/fix_ptrs.hpp>
#include <libprecisegc/details/allocators/core_allocator.hpp>
#include <libprecisegc/details/allocators/managed_pool_chunk.hpp>
#include <libprecisegc/details/allocators/cache_policies.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>

#include "test_forwarding.hpp"

using namespace precisegc::details;
using namespace precisegc::details::allocators;
using namespace precisegc::details::compacting;
using namespace precisegc::details::collectors;

namespace {
static const size_t OBJ_SIZE = 64;

struct test_type
{
    byte data[OBJ_SIZE];
};

typedef mso_allocator allocator_t;

}

static const type_meta* tmeta = type_meta_provider<test_type>::create_meta(std::vector<size_t>({0}));

struct fix_ptrs_test : public ::testing::Test
{
    allocator_t alloc;
};

TEST_F(fix_ptrs_test, test_fix_ptrs)
{
    gc_alloc_descriptor cell_ptr = alloc.allocate(OBJ_SIZE);
    cell_ptr.descriptor()->set_mark(cell_ptr.get(), true);
    cell_ptr.descriptor()->set_pin(cell_ptr.get(), false);
    byte* ptr = managed_object::get_object(cell_ptr.get());

    test_type val1;
    byte* to = reinterpret_cast<byte*>(&val1);

    test_type val2;
    byte*& from = * (byte**) ptr;
    from = reinterpret_cast<byte*>(&val2);

    traceable_object_meta* obj_meta = managed_object::get_meta(cell_ptr.get());
    new (obj_meta) traceable_object_meta(1, tmeta);
    obj_meta->set_forward_pointer(ptr);

    test_forwarding forwarding;
    forwarding.create(from, to, OBJ_SIZE);

    auto rng = alloc.memory_range();
    fix_ptrs(rng.begin(), rng.end(), forwarding, OBJ_SIZE);

    ASSERT_EQ(to, from);
}

