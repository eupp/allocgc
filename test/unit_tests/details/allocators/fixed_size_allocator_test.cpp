#include <gtest/gtest.h>

#include <memory>

#include "libprecisegc/details/allocators/fixed_size_allocator.h"
#include "libprecisegc/details/allocators/paged_allocator.h"
#include "libprecisegc/details/allocators/debug_layer.h"
#include "libprecisegc/details/allocators/types.h"

using namespace precisegc::details::allocators;

namespace {

static const size_t OBJ_SIZE = sizeof(size_t);

class test_chunk
{
public:
    typedef byte* pointer_type;

    test_chunk(byte* mem)
        : m_mem(mem)
        , m_available(true)
    {
        assert(mem);
    }

    test_chunk(const test_chunk&) = delete;
    test_chunk(test_chunk&&) = default;

    test_chunk& operator=(const test_chunk&) = delete;
    test_chunk& operator=(test_chunk&&) = default;

    byte* allocate(size_t obj_size)
    {
        assert(obj_size == OBJ_SIZE);
        assert(m_available);
        m_available = false;
        return m_mem;
    }

    void deallocate(byte* ptr, size_t obj_size)
    {
        assert(obj_size == OBJ_SIZE);
        assert(!m_available);
        assert(contains(ptr));
        m_available = true;
    }

    bool contains(byte* ptr) const noexcept
    {
        return ptr == m_mem;
    }

    bool memory_available() const noexcept
    {
        return m_available;
    }

    bool empty(size_t obj_size) const noexcept
    {
        assert(obj_size == OBJ_SIZE);
        return m_available;
    }

    template <typename Alloc>
    static test_chunk create(Alloc& allocator)
    {
        return test_chunk(allocator.allocate(OBJ_SIZE));
    }

    template <typename Alloc>
    static void destroy(test_chunk& chk, Alloc& allocator)
    {
        allocator.deallocate(chk.m_mem, OBJ_SIZE);
        chk.m_mem = nullptr;
        chk.m_available = false;
    }
private:
    byte* m_mem;
    bool m_available;
};

}

class fixed_size_allocator_test: public ::testing::Test
{
public:
    typedef debug_layer<paged_allocator> allocator_t;
    typedef fixed_size_allocator<test_chunk, allocator_t, allocator_t> fixed_allocator_t;

    fixed_allocator_t m_alloc;
};

TEST_F(fixed_size_allocator_test, test_allocate_1)
{
    size_t* ptr = (size_t*) m_alloc.allocate(OBJ_SIZE);
    ASSERT_NE(nullptr, ptr);
    *ptr = 42;

    m_alloc.deallocate((byte*) ptr, OBJ_SIZE);
}

TEST_F(fixed_size_allocator_test, test_allocate_2)
{
    byte* ptr = m_alloc.allocate(OBJ_SIZE);
    m_alloc.deallocate(ptr, OBJ_SIZE);

    ASSERT_EQ(0, m_alloc.get_allocator().get_allocated_mem_size());
}

TEST_F(fixed_size_allocator_test, test_allocate_3)
{
    size_t* ptr1 = (size_t*) m_alloc.allocate(OBJ_SIZE);
    size_t* ptr2 = (size_t*) m_alloc.allocate(OBJ_SIZE);

    ASSERT_NE(nullptr, ptr2);
    ASSERT_NE(ptr1, ptr2);
    *ptr2 = 42;

    m_alloc.deallocate((byte*) ptr1, OBJ_SIZE);
    m_alloc.deallocate((byte*) ptr2, OBJ_SIZE);
}