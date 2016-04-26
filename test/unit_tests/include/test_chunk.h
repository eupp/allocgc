#ifndef DIPLOMA_TEST_CHUNK_H
#define DIPLOMA_TEST_CHUNK_H

#include "libprecisegc/details/allocators/types.h"
#include "libprecisegc/details/allocators/iterator_range.h"
#include "libprecisegc/details/iterator_facade.h"
#include "libprecisegc/details/iterator_access.h"

class test_chunk
{
    typedef precisegc::details::byte byte;
public:
    typedef byte* pointer_type;

    static const size_t CHUNK_MINSIZE = 1;
    static const size_t CHUNK_MAXSIZE = 1;

    class iterator: public precisegc::details::iterator_facade<iterator, std::bidirectional_iterator_tag, byte* const>
    {
    public:
        typedef byte* const& reference;

        iterator() noexcept
            : m_ptr(nullptr)
            , m_obj_size(0)
        {}

        iterator(const iterator&) noexcept = default;
        iterator(iterator&&) noexcept = default;

        iterator& operator=(const iterator&) noexcept = default;
        iterator& operator=(iterator&&) noexcept = default;

        reference operator*() const noexcept
        {
            return m_ptr;
        }

        friend class test_chunk;
        friend class precisegc::details::iterator_access<iterator>;
    private:
        iterator(byte* ptr, size_t obj_size) noexcept
            : m_ptr(ptr)
            , m_obj_size(obj_size)
        {}

        bool equal(const iterator& other) const noexcept
        {
            return m_ptr == other.m_ptr;
        }

        void increment() noexcept
        {
            m_ptr += m_obj_size;
        }

        void decrement() noexcept
        {
            m_ptr -= m_obj_size;
        }

        byte* m_ptr;
        size_t m_obj_size;
    };

    typedef precisegc::details::allocators::iterator_range<iterator> range_type;

    test_chunk(byte* mem, size_t size, size_t obj_size)
            : m_mem(mem)
            , m_obj_size(obj_size)
            , m_available(true)
    {
        assert(mem);
        assert(size == obj_size);
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

    byte* get_mem() const
    {
        return m_mem;
    }

    size_t get_mem_size() const
    {
        return m_obj_size;
    }

    range_type get_range()
    {
        return range_type(iterator(m_mem, m_obj_size), iterator(m_mem + m_obj_size, m_obj_size));
    }
private:
    byte* m_mem;
    size_t m_obj_size;
    bool m_available;
};

#endif //DIPLOMA_TEST_CHUNK_H
