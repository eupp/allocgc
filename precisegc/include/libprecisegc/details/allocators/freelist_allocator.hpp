#ifndef DIPLOMA_FREELIST_ALLOCATOR_HPP
#define DIPLOMA_FREELIST_ALLOCATOR_HPP

#include <cassert>
#include <cstddef>
#include <type_traits>
#include <utility>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/utils/block_ptr.hpp>
#include <libprecisegc/gc_common.hpp>

namespace precisegc { namespace details { namespace allocators {

template <typename UpstreamAlloc>
class freelist_allocator : private utils::ebo<UpstreamAlloc>,
                           private utils::noncopyable, private utils::nonmovable
{
    struct control_block
    {
        control_block*  m_next;
        size_t          m_size;
    };
public:
    typedef typename UpstreamAlloc::pointer_type pointer_type;
    typedef typename UpstreamAlloc::memory_range_type memory_range_type;
    typedef stateful_alloc_tag alloc_tag;

    freelist_allocator()
        : m_head(nullptr)
    {}

    ~freelist_allocator()
    {
        shrink();
    }

    pointer_type allocate(size_t size)
    {
        assert(sizeof(control_block) <= size);
        if (m_head) {
            if (m_head->m_size >= size) {
                byte* ptr = reinterpret_cast<byte*>(m_head);
                m_head    = m_head->m_next;
                return ptr;
            }
            control_block* prev = m_head;
            control_block* curr = m_head->m_next;
            while (curr) {
                if (curr->m_size >= size) {
                    byte* ptr    = reinterpret_cast<byte*>(curr);
                    prev->m_next = curr->m_next;
                    return ptr;
                } else {
                    prev = curr;
                    curr = curr->m_next;
                }
            }
        }
        return upstream_allocate(size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        assert(ptr);
        assert(sizeof(control_block) <= size);
        control_block* head = reinterpret_cast<control_block*>(ptr);
        head->m_next = m_head;
        head->m_size = size;
        m_head = head;
    }

    size_t shrink()
    {
        size_t freed = 0;
        while (m_head) {
            control_block* next = m_head->m_next;
            freed += m_head->m_size;
            upstream_deallocate(reinterpret_cast<byte*>(m_head), m_head->m_size);
            m_head = next;
        }
        return freed;
    }

    const UpstreamAlloc& upstream_allocator() const
    {
        return this->template get_base<UpstreamAlloc>();
    }
private:
    byte* upstream_allocate(size_t size)
    {
        return this->template get_base<UpstreamAlloc>().allocate(size);
    }

    void upstream_deallocate(byte* ptr, size_t size)
    {
        this->template get_base<UpstreamAlloc>().deallocate(ptr, size);
    }

    control_block* m_head;
};

}}}

#endif //DIPLOMA_FREELIST_ALLOCATOR_HPP