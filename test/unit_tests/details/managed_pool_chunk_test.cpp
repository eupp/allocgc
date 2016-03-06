#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "libprecisegc/details/managed_pool_chunk.h"
#include "libprecisegc/details/allocators/paged_allocator.h"
#include "libprecisegc/details/allocators/debug_layer.h"

using namespace precisegc::details;
using namespace precisegc::details::allocators;

namespace {
const size_t CELL_SIZE = 64;
}

class managed_pool_chunk_test : public ::testing::Test
{
public:
    typedef debug_layer<paged_allocator> allocator_t;

    managed_pool_chunk_test()
        : m_chunk(managed_pool_chunk::create(CELL_SIZE, m_alloc))
    {}

    ~managed_pool_chunk_test()
    {
        if (m_chunk.get_mem()) {
            managed_pool_chunk::destroy(m_chunk, CELL_SIZE, m_alloc);
        }
    }

    allocator_t m_alloc;
    managed_pool_chunk m_chunk;
};

TEST_F(managed_pool_chunk_test, test_create)
{
    EXPECT_TRUE(m_chunk.memory_available());
    EXPECT_TRUE(m_chunk.empty(CELL_SIZE));
    EXPECT_NE(nullptr, m_chunk.get_descriptor());
}

TEST_F(managed_pool_chunk_test, test_destroy)
{
    managed_pool_chunk::destroy(m_chunk, CELL_SIZE, m_alloc);

    EXPECT_EQ(0, m_alloc.get_allocated_mem_size());
}

TEST_F(managed_pool_chunk_test, test_allocate)
{
    managed_cell_ptr cell_ptr = m_chunk.allocate(CELL_SIZE);

    EXPECT_NE(nullptr, cell_ptr.get());
    EXPECT_EQ(m_chunk.get_descriptor(), cell_ptr.get_wrapped().get_indexed_entry());
    EXPECT_TRUE(cell_ptr.owns_descriptor_lock());
}

