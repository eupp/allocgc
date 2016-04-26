#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "libprecisegc/details/managed_pool_chunk.h"
#include "libprecisegc/details/allocators/paged_allocator.h"
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
    typedef debug_layer<paged_allocator> allocator_t;

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
    EXPECT_TRUE(m_chunk.empty(CELL_SIZE));
    EXPECT_NE(nullptr, m_chunk.get_descriptor());
}

TEST_F(managed_pool_chunk_test, test_allocate)
{
    managed_cell_ptr cell_ptr = m_chunk.allocate(CELL_SIZE);

    EXPECT_NE(nullptr, cell_ptr.get());
    EXPECT_EQ(m_chunk.get_descriptor(), cell_ptr.get_wrapped().get_indexed_entry());
//    EXPECT_TRUE(cell_ptr.owns_descriptor_lock());
}

TEST_F(managed_pool_chunk_test, test_get_cell_meta)
{
    managed_memory_descriptor* descr = m_chunk.get_descriptor();
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
    managed_memory_descriptor* descr = m_chunk.get_descriptor();
    byte* mem = m_chunk.get_mem();
    size_t mem_size = m_chunk.get_mem_size();
    object_meta* meta = descr->get_cell_meta(mem);

    class_meta_provider<test_type>::create_meta(std::vector<size_t>());
    const class_meta* cls_meta = class_meta_provider<test_type>::get_meta_ptr();
    new (meta) object_meta(cls_meta, OBJ_CNT, mem);

    for (size_t i = 0; i < OBJ_CNT; ++i) {
        byte* obj_begin = mem + i * OBJ_SIZE;
        for (byte* p = obj_begin; p < obj_begin + OBJ_SIZE; ++p) {
            ASSERT_EQ(obj_begin, descr->get_object_begin(p));
        }
    }
}

TEST_F(managed_pool_chunk_test, test_get_mark_pin)
{
    managed_memory_descriptor* descr = m_chunk.get_descriptor();
    byte* mem = m_chunk.get_mem();
    size_t mem_size = m_chunk.get_mem_size();

    for (byte* ptr = mem; ptr < mem + mem_size; ++ptr) {
        ASSERT_FALSE(descr->get_mark(ptr));
        ASSERT_FALSE(descr->get_pin(ptr));
    }
}

TEST_F(managed_pool_chunk_test, test_set_mark)
{
    managed_memory_descriptor* descr = m_chunk.get_descriptor();
    byte* ptr = m_chunk.get_mem() + m_rand();

    descr->set_mark(ptr, true);
    ASSERT_EQ(descr->get_mark(ptr), true);
}

TEST_F(managed_pool_chunk_test, test_set_pin)
{
    managed_memory_descriptor* descr = m_chunk.get_descriptor();
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
        ASSERT_FALSE(cell_ptr.owns_descriptor_lock());
        it += CELL_SIZE;
    }
}