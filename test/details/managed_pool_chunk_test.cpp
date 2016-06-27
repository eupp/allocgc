#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "libprecisegc/details/allocators/managed_pool_chunk.hpp"
#include "libprecisegc/details/allocators/page_allocator.hpp"
#include "libprecisegc/details/allocators/debug_layer.h"

#include "rand_util.h"

using namespace precisegc::details;
using namespace precisegc::details::allocators;

namespace {
const size_t CELL_SIZE = 64;
const size_t CELL_COUNT = 64;
}

class managed_pool_chunk_test : public ::testing::Test
{
public:
    typedef debug_layer<page_allocator> allocator_t;

    managed_pool_chunk_test()
        : m_chunk(m_alloc.allocate(CELL_COUNT * CELL_SIZE), CELL_COUNT * CELL_SIZE, CELL_SIZE)
        , m_rand(0, PAGE_SIZE - 1)
    {}

    ~managed_pool_chunk_test()
    {
        if (m_chunk.get_mem()) {
            m_alloc.deallocate(m_chunk.get_mem(), m_chunk.get_mem_size());
        }
    }

    allocator_t m_alloc;
    managed_pool_chunk m_chunk;
    uniform_rand_generator<size_t> m_rand;
};

TEST_F(managed_pool_chunk_test, test_construct)
{
    EXPECT_TRUE(m_chunk.memory_available());
    EXPECT_TRUE(m_chunk.empty());
    EXPECT_NE(nullptr, m_chunk.get_descriptor());
}

TEST_F(managed_pool_chunk_test, test_allocate)
{
    managed_ptr cell_ptr = m_chunk.allocate(CELL_SIZE);

    EXPECT_NE(nullptr, cell_ptr.get());
    EXPECT_EQ(cell_ptr.get_descriptor(), m_chunk.get_descriptor());
    EXPECT_EQ(managed_ptr::index(cell_ptr.get()), m_chunk.get_descriptor());
}

TEST_F(managed_pool_chunk_test, test_get_cell_meta)
{
    memory_descriptor* descr = m_chunk.get_descriptor();
    byte* mem = m_chunk.get_mem();
    size_t mem_size = m_chunk.get_mem_size();

    for (byte* ptr = mem; ptr < mem + mem_size; ptr += CELL_SIZE) {
        object_meta* meta = object_meta::get_meta_ptr(ptr, CELL_SIZE);
        for (byte* p = ptr; p < ptr + CELL_SIZE; p++) {
            ASSERT_EQ(meta, descr->get_cell_meta(p));
        }
    }
}

namespace {
const size_t OBJ_SIZE = 8;
const size_t OBJ_CNT = 2;

static_assert(OBJ_CNT * OBJ_SIZE + sizeof(object_meta) <= CELL_SIZE,
              "managed_pool_chunk_test assertion failed");

struct test_type
{
    byte mem[OBJ_SIZE];
};
}

TEST_F(managed_pool_chunk_test, test_get_object_begin)
{
    memory_descriptor* descr = m_chunk.get_descriptor();
    byte* mem = m_chunk.get_mem();
    size_t mem_size = m_chunk.get_mem_size();
    object_meta* meta = descr->get_cell_meta(mem);

    type_meta_provider<test_type>::create_meta();
    const type_meta* cls_meta = type_meta_provider<test_type>::get_meta_ptr();
    new (meta) object_meta(cls_meta, OBJ_CNT, mem);

    for (size_t i = 0; i < OBJ_CNT; ++i) {
        byte* obj_begin = mem + i * OBJ_SIZE;
        for (byte* p = obj_begin; p < obj_begin + OBJ_SIZE; ++p) {
            ASSERT_EQ(obj_begin, descr->get_obj_begin(p));
        }
    }
}

TEST_F(managed_pool_chunk_test, test_get_mark_pin)
{
    memory_descriptor* descr = m_chunk.get_descriptor();
    byte* mem = m_chunk.get_mem();
    size_t mem_size = m_chunk.get_mem_size();

    for (byte* ptr = mem; ptr < mem + mem_size; ++ptr) {
        ASSERT_FALSE(descr->get_mark(ptr));
        ASSERT_FALSE(descr->get_pin(ptr));
    }
}

TEST_F(managed_pool_chunk_test, test_set_mark)
{
    memory_descriptor* descr = m_chunk.get_descriptor();
    byte* ptr = m_chunk.get_mem() + m_rand();

    descr->set_mark(ptr, true);
    ASSERT_EQ(descr->get_mark(ptr), true);
}

TEST_F(managed_pool_chunk_test, test_set_pin)
{
    memory_descriptor* descr = m_chunk.get_descriptor();
    byte* ptr = m_chunk.get_mem() + m_rand();

    descr->set_pin(ptr, true);
    ASSERT_EQ(descr->get_pin(ptr), true);
}

TEST_F(managed_pool_chunk_test, test_range)
{
    auto range = m_chunk.get_range();
    byte* it = m_chunk.get_mem();
    for (auto cell_ptr: range) {
        ASSERT_EQ(it, cell_ptr.get());
        it += CELL_SIZE;
    }
}