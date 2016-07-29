#ifndef DIPLOMA_FREELIST_ALLOCATOR_HPP
#define DIPLOMA_FREELIST_ALLOCATOR_HPP

#include <cstddef>

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/types.hpp>
#include <boost/range/iterator_range.hpp>
#include <libprecisegc/details/utils/locked_range.hpp>

namespace precisegc { namespace details { namespace allocators {

template <typename Descriptor, typename UpstreamAlloc, typename Lock>
class freelist_allocator : private utils::ebo<UpstreamAlloc>, private utils::noncopyable, private utils::nonmovable
{
public:
    typedef typename Descriptor::pointer_type pointer_type;
    typedef stateful_alloc_tag alloc_tag;

    struct control_block
    {
        byte*   m_next;
        byte*   m_prev;
        size_t  m_size;
    };

    class iterator: public boost::iterator_facade<
              iterator
            , const pointer_type
            , boost::bidirectional_traversal_tag
            , pointer_type
        >
    {
    public:
        class proxy
        {
        public:
            proxy(const pointer_type& ptr)
                : m_ptr(ptr)
            {}

            const pointer_type* operator->()
            {
                return &m_ptr;
            }
        private:
            pointer_type m_ptr;
        };

        iterator(const iterator&) noexcept = default;
        iterator(iterator&&) noexcept = default;

        iterator& operator=(const iterator&) noexcept = default;
        iterator& operator=(iterator&&) noexcept = default;

        proxy operator->() const
        {
            return proxy(get_descriptor(m_memblk)->get_mem());
        }
    private:
        friend class freelist_allocator;
        friend class boost::iterator_core_access;

        iterator(byte* memblk) noexcept
            : m_memblk(memblk)
        {}

        pointer_type dereference() const
        {
            return get_descriptor(m_memblk).get_mem();
        }

        void increment() noexcept
        {
            m_memblk = get_next_memblk(m_memblk);
        }

        void decrement() noexcept
        {
            m_memblk = get_prev_memblk(m_memblk);
        }

        bool equal(const iterator& other) const noexcept
        {
            return m_memblk == other.m_memblk;
        }

        byte* m_memblk;
    };

    typedef utils::locked_range<
              boost::iterator_range<iterator>
            , Lock
        > memory_range_type;

    pointer_type allocate(size_t size)
    {

    }

    void deallocate(pointer_type ptr, size_t size)
    {

    }

    size_t shrink()
    {

    }

    template <typename Functor>
    void apply_to_descriptors(Functor&& f)
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        for (byte* blk = m_head; blk != get_fake_block(); blk = get_next_memblk(blk))
        {
            f(get_descriptor(blk));
        }
    }

    memory_range_type memory_range()
    {
        std::unique_lock<Lock> lock_guard(m_lock);
        return memory_range_type(boost::make_iterator_range(begin(), end()), std::move(lock_guard));
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

    iterator begin()
    {
        return iterator(m_head);
    }

    iterator end()
    {
        return iterator(get_fake_block());
    }

    byte* get_fake_block()
    {
        return reinterpret_cast<byte*>(&m_fake);
    }

    static byte* get_next_memblk(byte* memblk)
    {
        return get_control_block(memblk).m_next;
    }

    static byte* get_prev_memblk(byte* memblk)
    {
        return get_control_block(memblk).m_prev;
    }

    static control_block& get_control_block(byte* memblk)
    {
        return *reinterpret_cast<control_block*>(memblk);
    }

    static Descriptor& get_descriptor(byte* memblk)
    {
        return *reinterpret_cast<Descriptor*>(memblk + sizeof(control_block));
    }

    static byte* get_mem(byte* memblk)
    {
        return memblk + sizeof(control_block) + sizeof(Descriptor);
    }

    control_block m_fake;
    byte* m_head;
    Lock m_lock;
};

}}}

#endif //DIPLOMA_FREELIST_ALLOCATOR_HPP
