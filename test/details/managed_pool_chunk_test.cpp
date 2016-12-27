#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include <libprecisegc/details/collectors/memory_index.hpp>
#include <libprecisegc/details/allocators/managed_pool_chunk.hpp>
#include <libprecisegc/details/allocators/core_allocator.hpp>
#include <libprecisegc/details/allocators/debug_layer.hpp>

#include "rand_util.h"

using namespace precisegc::details;
using namespace precisegc::details::allocators;
using namespace precisegc::details::collectors;

namespace {
const size_t CELL_SIZE  = 64;
const size_t CELL_COUNT = 64;
const size_t CHUNK_SIZE = managed_pool_chunk::chunk_size(CELL_COUNT * CELL_SIZE);
}

class managed_pool_chunk_test : public ::testing::Test
{
public:
    typedef debug_layer<core_allocator> allocator_t;

    managed_pool_chunk_test()
        : m_chunk(m_alloc.allocate(CHUNK_SIZE), CELL_COUNT * CELL_SIZE, CELL_SIZE)
        , m_rand(0, CELL_COUNT)
    {}

    ~managed_pool_chunk_test()
    {
        if (m_chunk.memory()) {
            m_alloc.deallocate(m_chunk.memory(), CHUNK_SIZE);
        }
    }

    allocator_t m_alloc;
    managed_pool_chunk m_chunk;
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
    memory_descriptor* descr = m_chunk.descriptor();
    byte* mem = m_chunk.memory();
    size_t mem_size = m_chunk.size();

    for (byte* ptr = mem; ptr < mem + mem_size; ptr += CELL_SIZE) {
        for (byte* p = ptr; p < ptr + CELL_SIZE; p++) {
            ASSERT_EQ(ptr, descr->cell_start(p));
        }
    }
}

namespace {
const size_t OBJ_SIZE = 8;
const size_t OBJ_CNT = 2;

static_assert(OBJ_CNT * OBJ_SIZE + sizeof(traceable_object_meta) <= CELL_SIZE,
              "managed_pool_chunk_test assertion failed");

struct test_type
{
    byte mem[OBJ_SIZE];
};
}

TEST_F(managed_pool_chunk_test, test_get_mark_pin)
{
    memory_descriptor* descr = m_chunk.descriptor();
    byte* mem = m_chunk.memory();
    size_t mem_size = m_chunk.size();

    for (byte* ptr = mem; ptr < mem + mem_size; ptr += CELL_SIZE) {
        byte* obj = ptr + sizeof(collectors::traceable_object_meta);
        ASSERT_FALSE(descr->get_mark(obj));
        ASSERT_FALSE(descr->get_pin(obj));
    }
}

TEST_F(managed_pool_chunk_test, test_set_mark)
{
    memory_descriptor* descr = m_chunk.descriptor();
    byte* ptr = m_chunk.memory() + CELL_SIZE * m_rand() + sizeof(collectors::traceable_object_meta);

    descr->set_mark(ptr, true);
    ASSERT_EQ(descr->get_mark(ptr), true);
}

TEST_F(managed_pool_chunk_test, test_set_pin)
{
    memory_descriptor* descr = m_chunk.descriptor();
    byte* ptr = m_chunk.memory() + CELL_SIZE * m_rand() + sizeof(collectors::traceable_object_meta);

    descr->set_pin(ptr, true);
    ASSERT_EQ(descr->get_pin(ptr), true);
}

TEST_F(managed_pool_chunk_test, test_range)
{
    auto range = m_chunk.memory_range();
    byte* it = m_chunk.memory();
    for (auto cell_ptr: range) {
        ASSERT_EQ(it, cell_ptr.get());
        it += CELL_SIZE;
    }
}