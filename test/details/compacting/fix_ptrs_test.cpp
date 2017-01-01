#include <gtest/gtest.h>

#include <libprecisegc/details/gc_type_meta_factory.hpp>
#include <libprecisegc/details/compacting/fix_ptrs.hpp>
#include <libprecisegc/details/allocators/gc_pool_allocator.hpp>

#include "test_forwarding.hpp"

using namespace precisegc::details;
using namespace precisegc::details::allocators;
using namespace precisegc::details::compacting;

namespace {
static const size_t OBJ_SIZE = 32;

struct test_type
{
    byte data[OBJ_SIZE];
};

typedef gc_pool_allocator allocator_t;

}

static const gc_type_meta* tmeta = gc_type_meta_factory<test_type>::create(std::vector<size_t>({0}));

struct fix_ptrs_test : public ::testing::Test
{
    allocator_t alloc;
};

TEST_F(fix_ptrs_test, test_fix_ptrs)
{
    gc_alloc_response rsp = alloc.allocate(gc_alloc_request(OBJ_SIZE, 1, tmeta), gc_box::box_size(OBJ_SIZE));
    rsp.set_mark(true);
    rsp.set_pin(false);
    byte* ptr = rsp.obj_start();

    test_type val1;
    byte* to = reinterpret_cast<byte*>(&val1);

    test_type val2;
    byte*& from = * (byte**) ptr;
    from = reinterpret_cast<byte*>(&val2);

    test_forwarding forwarding;
    forwarding.create(from, to);

    auto rng = alloc.memory_range();
    fix_ptrs(rng.begin(), rng.end(), forwarding);

    ASSERT_EQ(to, from);
}

