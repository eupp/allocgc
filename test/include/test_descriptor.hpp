#ifndef DIPLOMA_TEST_DESCRIPTOR_HPP
#define DIPLOMA_TEST_DESCRIPTOR_HPP

#include <cstddef>
#include <cassert>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/types.hpp>

class test_descriptor : private precisegc::details::utils::noncopyable,
                        private precisegc::details::utils::nonmovable
{
    typedef precisegc::details::byte byte;
public:
    typedef precisegc::details::byte* pointer_type;

    static size_t align_size(size_t size)
    {
        return size;
    }

    test_descriptor(byte* mem, size_t size)
        : m_mem(mem)
        , m_empty(false)
    {
        assert(mem);
        assert(size > 0);
    }

    byte* get_mem() const
    {
        return m_mem;
    }

    bool empty() const
    {
        return m_empty;
    }

    void set_empty(bool value)
    {
        m_empty = value;
    }
private:
    byte* m_mem;
    bool  m_empty;
};

#endif //DIPLOMA_TEST_DESCRIPTOR_HPP
