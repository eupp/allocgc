#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include <liballocgc/details/allocators/memory_index.hpp>
#include <liballocgc/details/allocators/gc_pool_descriptor.hpp>
#include <liballocgc/details/allocators/gc_core_allocator.hpp>
#include <liballocgc/details/allocators/debug_layer.hpp>
#include <liballocgc/details/allocators/gc_box.hpp>

#include "rand_util.h"

using namespace allocgc;
using namespace allocgc::details;
using namespace allocgc::details::allocators;

namespace {
const size_t CELL_SIZE  = 64;
const size_t CELL_COUNT = 64;
const size_t CHUNK_SIZE = gc_pool_descriptor::chunk_size(CELL_COUNT * CELL_SIZE);
}

class managed_pool_chunk_test : public ::testing::Test
{
public:
    typedef debug_layer<gc_core_allocator> allocator_t;

    managed_pool_chunk_test()
        : m_chunk(m_alloc.allocate(CHUNK_SIZE), CELL_COUNT * CELL_SIZE, CELL_SIZE, m_bucket_policy)
        , m_rand(0, CELL_COUNT)
    {}

    ~managed_pool_chunk_test()
    {
        if (m_chunk.memory()) {
            m_alloc.deallocate(m_chunk.memory(), CHUNK_SIZE);
        }
    }

    allocator_t m_alloc;
    gc_bucket_policy m_bucket_policy;
    gc_pool_descriptor m_chunk;
    uniform_rand_generator<size_t> m_rand;
};

//TEST_F(managed_pool_chunk_test, test_construct)
//{
//    EXPECT_TRUE(m_chunk.memory_available());
//    EXPECT_TRUE(m_chunk.unused());
//    EXPECT_NE(nullptr, m_chunk.descriptor());
//}
//
//TEST_F(managed_pool_chunk_test, test_allocate)
//{
//    gc_alloc_response cell_ptr = m_chunk.allocate(CELL_SIZE);
//
//    EXPECT_NE(nullptr, cell_ptr.get());
//    EXPECT_EQ(cell_ptr.descriptor(), m_chunk.descriptor());
//    EXPECT_EQ(memory_index::index_memory(cell_ptr.get()), m_chunk.descriptor());
//}

TEST_F(managed_pool_chunk_test, test_cell_start)
{
    gc_memory_descriptor* descr = m_chunk.descriptor();
    byte* mem = m_chunk.memory();
    size_t mem_size = m_chunk.size();

    for (byte* ptr = mem; ptr < mem + mem_size; ptr += CELL_SIZE) {
        for (byte* p = ptr; p < ptr + CELL_SIZE; p++) {
            ASSERT_EQ(ptr, descr->box_addr(descr->get_id(p)));
        }
    }
}

namespace {
const size_t OBJ_SIZE = 8;
const size_t OBJ_CNT = 2;

static_assert(gc_box::box_size(OBJ_CNT * OBJ_SIZE) <= CELL_SIZE,
              "managed_pool_chunk_test assertion failed");

struct test_type
{
    byte mem[OBJ_SIZE];
};
}

TEST_F(managed_pool_chunk_test, test_get_mark_pin)
{
    gc_memory_descriptor* descr = m_chunk.descriptor();
    byte* mem = m_chunk.memory();
    size_t mem_size = m_chunk.size();

    for (byte* ptr = mem; ptr < mem + mem_size; ptr += CELL_SIZE) {
        ASSERT_FALSE(descr->get_mark(descr->get_id(ptr)));
        ASSERT_FALSE(descr->get_pin(descr->get_id(ptr)));
    }
}

TEST_F(managed_pool_chunk_test, test_set_mark)
{
    gc_memory_descriptor* descr = m_chunk.descriptor();
    byte* ptr = m_chunk.memory() + CELL_SIZE * m_rand();

    descr->set_mark(descr->get_id(ptr), true);
    ASSERT_EQ(descr->get_mark(descr->get_id(ptr)), true);
}

TEST_F(managed_pool_chunk_test, test_set_pin)
{
    gc_memory_descriptor* descr = m_chunk.descriptor();
    byte* ptr = m_chunk.memory() + CELL_SIZE * m_rand();

    descr->set_pin(descr->get_id(ptr), true);
    ASSERT_EQ(descr->get_pin(descr->get_id(ptr)), true);
}

//TEST_F(managed_pool_chunk_test, test_range)
//{
//    auto range = m_chunk.memory_range();
//    byte* it = m_chunk.memory();
//    for (auto cell_ptr: range) {
//        ASSERT_EQ(it, cell_ptr.get());
//        it += CELL_SIZE;
//    }
//}