#ifndef ALLOCGC_REDIRECTION_ALLOCATOR_HPP
#define ALLOCGC_REDIRECTION_ALLOCATOR_HPP

#include <cstddef>

#include <boost/range/iterator_range.hpp>

#include <liballocgc/details/allocators/allocator_tag.hpp>
#include <liballocgc/gc_common.hpp>

namespace allocgc { namespace details { namespace allocators {

template <typename Alloc>
class redirection_allocator
{
public:
    typedef stateful_alloc_tag alloc_tag;

    typedef typename Alloc::pointer_type pointer_type;
    typedef typename Alloc::memory_range_type memory_range_type;

    redirection_allocator()
        : m_alloc(nullptr)
    {}

    explicit redirection_allocator(Alloc* alloc)
        : m_alloc(alloc)
    {}

    redirection_allocator(const redirection_allocator&) = default;
    redirection_allocator(redirection_allocator&&) = default;

    redirection_allocator& operator=(const redirection_allocator&) = default;
    redirection_allocator& operator=(redirection_allocator&&) = default;

    byte* allocate(size_t size)
    {
        assert(m_alloc);
        return m_alloc->allocate(size);
    }

    void deallocate(byte* ptr, size_t size)
    {
        assert(m_alloc);
        m_alloc->deallocate(ptr, size);
    }

    memory_range_type memory_range()
    {
        assert(m_alloc);
        return m_alloc->memory_range();
    }

    Alloc* allocator() const
    {
        return m_alloc;
    }
private:
    Alloc* m_alloc;
};

}}}

#endif //ALLOCGC_REDIRECTION_ALLOCATOR_HPP
