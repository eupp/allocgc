#include <gtest/gtest.h>

#include <liballocgc/details/allocators/gc_lo_allocator.hpp>
#include <liballocgc/gc_type_meta.hpp>

#include "utils.hpp"

using namespace allocgc;
using namespace allocgc::details;
using namespace allocgc::details::allocators;

namespace {
static const size_t OBJ_SIZE = 64;

struct test_type
{ };

const gc_type_meta* type_meta = gc_type_meta_factory<test_type>::create();
}

struct gc_lo_allocator_test : public ::testing::Test
{
    gc_lo_allocator_test()
        : alloc(&core_alloc)
        , rqst(OBJ_SIZE, 1, nullptr, &buf)
    {}

    gc_core_allocator core_alloc;
    gc_lo_allocator alloc;
    gc_buf buf;
    gc_alloc::request rqst;
};

TEST_F(gc_lo_allocator_test, test_allocate_1)
{
    gc_alloc::response rsp = alloc.allocate(rqst);
    commit(rsp, type_meta);

    ASSERT_NE(nullptr, rsp.obj_start());
    ASSERT_LE(OBJ_SIZE, rsp.cell_size());
}

TEST_F(gc_lo_allocator_test, test_allocate_2)
{
    gc_alloc::response rsp1 = alloc.allocate(rqst);
    commit(rsp1, type_meta);

    gc_alloc::response rsp2 = alloc.allocate(rqst);
    commit(rsp2, type_meta);

    ASSERT_NE(nullptr, rsp1.obj_start());
    ASSERT_NE(nullptr, rsp2.obj_start());
    ASSERT_NE(rsp1.obj_start(), rsp2.obj_start());
}

TEST_F(gc_lo_allocator_test, test_collect)
{
    gc_alloc::response rsp1 = alloc.allocate(rqst);
    commit(rsp1, type_meta);
    set_mark(rsp1, true);
    set_pin(rsp1, true);

    gc_alloc::response rsp2 = alloc.allocate(rqst);
    commit(rsp2, type_meta);

    gc_alloc::response rsp3 = alloc.allocate(rqst);
    commit(rsp3, type_meta);
    set_mark(rsp3, true);


    compacting::forwarding frwd;
    gc_heap_stat stat = alloc.collect(frwd);

    ASSERT_EQ(3 * OBJ_SIZE, stat.mem_before_gc);
    ASSERT_EQ(2 * OBJ_SIZE, stat.mem_occupied);
    ASSERT_EQ(2 * OBJ_SIZE, stat.mem_live);
    ASSERT_EQ(OBJ_SIZE, stat.mem_freed);
    ASSERT_EQ(0, stat.mem_copied);
    ASSERT_EQ(1, stat.pinned_cnt);
}