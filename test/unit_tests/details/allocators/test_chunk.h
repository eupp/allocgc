#ifndef DIPLOMA_TEST_CHUNK_H
#define DIPLOMA_TEST_CHUNK_H

#include "libprecisegc/details/allocators/types.h"

class test_chunk
{
    typedef precisegc::details::allocators::byte byte;
public:
    typedef byte* pointer_type;

    test_chunk(byte* mem, size_t obj_size)
            : m_mem(mem)
            , m_obj_size(obj_size)
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
        assert(obj_size == m_obj_size);
        if (m_available) {
            m_available = false;
            return m_mem;
        }
        return nullptr;
    }

    void deallocate(byte* ptr, size_t obj_size)
    {
        assert(obj_size == m_obj_size);
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
        assert(obj_size == m_obj_size);
        return m_available;
    }

    template <typename Alloc>
    static test_chunk create(size_t obj_size, Alloc& allocator)
    {
        return test_chunk(allocator.allocate(obj_size), obj_size);
    }

    template <typename Alloc>
    static void destroy(test_chunk& chk, size_t obj_size, Alloc& allocator)
    {
        allocator.deallocate(chk.m_mem, obj_size);
        chk.m_mem = nullptr;
        chk.m_available = false;
    }
private:
    byte* m_mem;
    size_t m_obj_size;
    bool m_available;
};

#endif //DIPLOMA_TEST_CHUNK_H
