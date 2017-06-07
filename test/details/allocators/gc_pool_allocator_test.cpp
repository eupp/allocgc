#include <gtest/gtest.h>

#include <liballocgc/details/allocators/gc_pool_allocator.hpp>
#include <liballocgc/gc_type_meta.hpp>
#include <liballocgc/details/allocators/gc_bucket_policy.hpp>

#include "utils.hpp"

using namespace allocgc;
using namespace allocgc::details;
using namespace allocgc::details::allocators;

namespace {
static const size_t OBJ_SIZE = 16;
static const size_t ALLOC_SIZE = gc_box::box_size(OBJ_SIZE);
static const size_t CHUNK_SIZE = GC_POOL_CHUNK_OBJECTS_COUNT * ALLOC_SIZE;

struct test_type
{
    byte data[OBJ_SIZE];
};

const gc_type_meta* type_meta = gc_type_meta_factory<test_type>::create();
}

struct gc_pool_allocator_test : public ::testing::Test
{
    gc_pool_allocator_test()
        : rqst(OBJ_SIZE, 1, type_meta, &buf)
    {
        alloc.set_core_allocator(&core_alloc);
        alloc.set_offset_table(bucket_policy.offsets_table(bucket_policy.bucket_id(ALLOC_SIZE)));
    }

    gc_bucket_policy bucket_policy;
    gc_core_allocator core_alloc;
    gc_pool_allocator alloc;
    gc_buf buf;
    gc_alloc::request rqst;
};

TEST_F(gc_pool_allocator_test, test_allocate_1)
{
    gc_alloc::response rsp = alloc.allocate(rqst, ALLOC_SIZE);
    commit(rsp, type_meta);

    ASSERT_NE(nullptr, rsp.obj_start());
    ASSERT_LE(OBJ_SIZE, rsp.cell_size());
}

TEST_F(gc_pool_allocator_test, test_allocate_2)
{
    gc_alloc::response rsp1 = alloc.allocate(rqst, ALLOC_SIZE);
    commit(rsp1, type_meta);

    gc_alloc::response rsp2 = alloc.allocate(rqst, ALLOC_SIZE);
    commit(rsp2, type_meta);

    ASSERT_NE(nullptr, rsp1.obj_start());
    ASSERT_NE(nullptr, rsp2.obj_start());
    ASSERT_NE(rsp1.obj_start(), rsp2.obj_start());
}

TEST_F(gc_pool_allocator_test, test_collect)
{
    gc_alloc::response rsp1 = alloc.allocate(rqst, ALLOC_SIZE);
    commit(rsp1);
    set_mark(rsp1, true);
    set_pin(rsp1, true);

    gc_alloc::response rsp2 = alloc.allocate(rqst, ALLOC_SIZE);
    commit(rsp2);

    gc_alloc::response rsp3 = alloc.allocate(rqst, ALLOC_SIZE);
    commit(rsp3);
    set_mark(rsp3, true);

    ASSERT_EQ(CHUNK_SIZE, alloc.stats().mem_used);

    compacting::forwarding frwd;
    gc_collect_stat stat = alloc.collect(frwd);
    gc_memstat memstat = alloc.stats();

    ASSERT_GE(CHUNK_SIZE, memstat.mem_used);
    ASSERT_EQ(2 * OBJ_SIZE, memstat.mem_live);
    ASSERT_EQ(ALLOC_SIZE, stat.mem_freed);
//    ASSERT_EQ(ALLOC_SIZE, stat.mem_copied);
    ASSERT_EQ(1, stat.pinned_cnt);
}
