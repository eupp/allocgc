#include <gtest/gtest.h>

#include <libprecisegc/details/allocators/gc_pool_allocator.hpp>
#include <libprecisegc/details/gc_type_meta_factory.hpp>

using namespace precisegc;
using namespace precisegc::details;
using namespace precisegc::details::allocators;

namespace {
static const size_t OBJ_SIZE = 16;
static const size_t ALLOC_SIZE = gc_box::box_size(OBJ_SIZE);
static const size_t CHUNK_SIZE = MANAGED_CHUNK_OBJECTS_COUNT * ALLOC_SIZE;

struct test_type
{ };

const gc_type_meta* type_meta = gc_type_meta_factory<test_type>::create();
}

struct gc_pool_allocator_test : public ::testing::Test
{
    gc_pool_allocator_test()
        : rqst(OBJ_SIZE, 1, type_meta)
    {}

    gc_pool_allocator alloc;
    gc_alloc_request rqst;
};

TEST_F(gc_pool_allocator_test, test_allocate_1)
{
    gc_alloc_response rsp = alloc.allocate(rqst, ALLOC_SIZE);
    rsp.commit(type_meta);

    ASSERT_NE(nullptr, rsp.obj_start());
    ASSERT_LE(OBJ_SIZE, rsp.size());
    ASSERT_NE(nullptr, rsp.descriptor());
}

TEST_F(gc_pool_allocator_test, test_allocate_2)
{
    gc_alloc_response rsp1 = alloc.allocate(rqst, ALLOC_SIZE);
    gc_alloc_response rsp2 = alloc.allocate(rqst, ALLOC_SIZE);
    rsp1.commit(type_meta);
    rsp2.commit(type_meta);

    ASSERT_NE(nullptr, rsp1.obj_start());
    ASSERT_NE(nullptr, rsp2.obj_start());
    ASSERT_NE(rsp1.obj_start(), rsp2.obj_start());
}

TEST_F(gc_pool_allocator_test, test_collect)
{
    gc_alloc_response rsps[3];
    for (auto& rsp: rsps) {
        rsp = alloc.allocate(rqst, ALLOC_SIZE);
        rsp.commit();
    }

    rsps[0].set_mark(true);
    rsps[0].set_pin(true);
    rsps[2].set_mark(true);

    compacting::forwarding frwd;
    gc_heap_stat stat = alloc.collect(frwd);

    ASSERT_EQ(CHUNK_SIZE, stat.mem_before_gc);
    ASSERT_GE(CHUNK_SIZE, stat.mem_all);
    ASSERT_EQ(2 * ALLOC_SIZE, stat.mem_live);
    ASSERT_EQ(ALLOC_SIZE, stat.mem_freed);
//    ASSERT_EQ(ALLOC_SIZE, stat.mem_copied);
    ASSERT_EQ(1, stat.pinned_cnt);
}
