#ifndef DIPLOMA_FIXSIZE_FREELIST_ALLOCATOR_HPP
#define DIPLOMA_FIXSIZE_FREELIST_ALLOCATOR_HPP

#include <cassert>
#include <cstddef>
#include <utility>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace allocators {

template <typename UpstreamAlloc>
class fixsize_freelist_allocator : private utils::ebo<UpstreamAlloc>,
                                   private utils::noncopyable, private utils::nonmovable
{
public:
    typedef typename UpstreamAlloc::pointer_type pointer_type;
    typedef typename UpstreamAlloc::memory_range_type memory_range_type;
    typedef stateful_alloc_tag alloc_tag;

    fixsize_freelist_allocator() = default;

    pointer_type allocate(size_t size)
    {
        assert(sizeof(pointer_type) <= size);
        if (m_head) {
            pointer_type ptr = m_head;
            m_head = *reinterpret_cast<pointer_type*>(m_head.get());
            return ptr;
        }
        return mutable_upstream_allocator().allocate(size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        assert(ptr);
        assert(sizeof(pointer_type) <= size);
        pointer_type* next = reinterpret_cast<pointer_type*>(ptr.get());
        *next = m_head;
        m_head = ptr;
    }

    // definitely not a good solutin:
    // we define two versions of shrink() with size argument and without;
    // second version expects to get size from pointer_type::size() method of m_head;
    // clearly, SFINAE should be used here but we're too lazy

    size_t shrink()
    {
        return shrink(m_head.size());
    }

    size_t shrink(size_t size)
    {
        size_t freed = 0;
        while (m_head) {
            pointer_type* next = reinterpret_cast<pointer_type*>(m_head.get());
            mutable_upstream_allocator().deallocate(m_head, size);
            m_head = next;
        }
        return mutable_upstream_allocator().shrink();
    }

    template <typename Functor>
    void apply_to_chunks(Functor&& f)
    {
        mutable_upstream_allocator().apply_to_chunks(std::forward<Functor>(f));
    }

    memory_range_type memory_range()
    {
        return mutable_upstream_allocator().memory_range();
    }

    const UpstreamAlloc& upstream_allocator() const
    {
        return this->template get_base<UpstreamAlloc>();
    }
private:
    UpstreamAlloc& mutable_upstream_allocator()
    {
        return this->template get_base<UpstreamAlloc>();
    }

    pointer_type m_head;
};

}}}

#endif //DIPLOMA_FIXSIZE_FREELIST_ALLOCATOR_HPP
