#include <gtest/gtest.h>

#include <libprecisegc/details/allocators/gc_lo_allocator.hpp>

using namespace precisegc;
using namespace precisegc::details;
using namespace precisegc::details::allocators;

namespace {
static const size_t OBJ_SIZE = 64;
}

struct gc_lo_allocator_test : public ::testing::Test
{
    gc_lo_allocator_test()
        : rqst(OBJ_SIZE, 1, nullptr)
    {}

    gc_lo_allocator alloc;
    gc_alloc_request rqst;
};

TEST_F(gc_lo_allocator_test, test_allocate_1)
{
    gc_alloc_response rsp = alloc.allocate(rqst);

    ASSERT_NE(nullptr, rsp.get());
    ASSERT_LE(OBJ_SIZE, rsp.size());
    ASSERT_NE(nullptr, rsp.descriptor());
}

TEST_F(gc_lo_allocator_test, test_allocate_2)
{
    gc_alloc_response rsp1 = alloc.allocate(rqst);
    gc_alloc_response rsp2 = alloc.allocate(rqst);

    ASSERT_NE(nullptr, rsp1.get());
    ASSERT_NE(nullptr, rsp2.get());
    ASSERT_NE(rsp1.get(), rsp2.get());
}

TEST_F(gc_lo_allocator_test, test_collect)
{
    gc_alloc_response rsps[3];
    for (auto& rsp: rsps) {
        rsp = alloc.allocate(rqst);
    }

    rsps[0].set_mark(true);
    rsps[0].set_pin(true);
    rsps[2].set_mark(true);

    compacting::forwarding frwd;
    gc_heap_stat stat = alloc.collect(frwd);

    ASSERT_EQ(3 * OBJ_SIZE, stat.mem_before_gc);
    ASSERT_EQ(2 * OBJ_SIZE, stat.mem_all);
    ASSERT_EQ(2 * OBJ_SIZE, stat.mem_live);
    ASSERT_EQ(OBJ_SIZE, stat.mem_freed);
    ASSERT_EQ(0, stat.mem_copied);
    ASSERT_EQ(1, stat.pinned_cnt);
}