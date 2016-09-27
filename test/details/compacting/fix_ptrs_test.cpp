#include <gtest/gtest.h>

#include <libprecisegc/details/compacting/fix_ptrs.hpp>
#include <libprecisegc/details/allocators/list_allocator.hpp>
#include <libprecisegc/details/allocators/core_allocator.hpp>
#include <libprecisegc/details/allocators/managed_pool_chunk.hpp>
#include <libprecisegc/details/allocators/cache_policies.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>

#include "test_forwarding.hpp"

using namespace precisegc::details;
using namespace precisegc::details::allocators;
using namespace precisegc::details::compacting;

namespace {
static const size_t OBJ_SIZE = 64;

struct test_type
{
    byte data[OBJ_SIZE];
};

typedef list_allocator<managed_pool_chunk,
        core_allocator,
        default_allocator,
        single_chunk_cache,
        utils::dummy_mutex
    > allocator_t;

}

static const type_meta* tmeta = type_meta_provider<test_type>::create_meta(std::vector<size_t>({0}));

struct fix_ptrs_test : public ::testing::Test
{
    allocator_t alloc;
};

TEST_F(fix_ptrs_test, test_fix_ptrs)
{
    managed_ptr cell_ptr = alloc.allocate(OBJ_SIZE);
    cell_ptr.set_mark(true);
    cell_ptr.set_pin(false);
    byte* ptr = object_meta::get_object_ptr(cell_ptr.get(), cell_ptr.cell_size());

    test_type val1;
    byte* to = reinterpret_cast<byte*>(&val1);

    test_type val2;
    byte*& from = * (byte**) ptr;
    from = reinterpret_cast<byte*>(&val2);

    object_meta* obj_meta = object_meta::get_meta_ptr(cell_ptr.get(), cell_ptr.cell_size());
    new (obj_meta) object_meta(1, tmeta);
    obj_meta->set_forward_pointer(ptr);

    test_forwarding forwarding;
    forwarding.create(from, to, OBJ_SIZE);

    auto rng = alloc.memory_range();
    fix_ptrs(rng.begin(), rng.end(), forwarding, OBJ_SIZE);

    ASSERT_EQ(to, from);
}

