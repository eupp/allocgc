#include <gtest/gtest.h>

#include <liballocgc/gc_type_meta.hpp>
#include <liballocgc/details/compacting/fix_ptrs.hpp>
#include <liballocgc/details/allocators/gc_pool_allocator.hpp>

#include "test_forwarding.hpp"
#include "utils.hpp"

using namespace allocgc;
using namespace allocgc::details;
using namespace allocgc::details::allocators;
using namespace allocgc::details::compacting;

namespace {
static const size_t OBJ_SIZE = gc_box::obj_size(32);

struct test_type
{
    byte data[OBJ_SIZE];
};

}

static const gc_type_meta* type_meta = gc_type_meta_factory<test_type>::create(std::vector<size_t>({0}));

struct fix_ptrs_test : public ::testing::Test
{
    fix_ptrs_test()
    {
        alloc.set_core_allocator(&core_alloc);
    }

    gc_core_allocator core_alloc;
    gc_pool_allocator alloc;
};

//TEST_F(fix_ptrs_test, test_fix_ptrs)
//{
//    gc_buf buf;
//    gc_alloc::response rsp = alloc.allocate(gc_alloc::request(OBJ_SIZE, 1, type_meta, &buf), gc_box::box_size(OBJ_SIZE));
//
//    commit(rsp, type_meta);
//    set_mark(rsp, true);
//    set_pin(rsp, false);
//    byte* ptr = rsp.obj_start();
//
//    test_type val1;
//    byte* to = reinterpret_cast<byte*>(&val1);
//
//    test_type val2;
//    byte*& from = * (byte**) ptr;
//    from = reinterpret_cast<byte*>(&val2);
//
//    test_forwarding forwarding;
//    forwarding.create(from, to);
//
//    auto rng = alloc.memory_range();
//    fix_ptrs(rng.begin(), rng.end(), forwarding);
//
//    ASSERT_EQ(to, from);
//}

