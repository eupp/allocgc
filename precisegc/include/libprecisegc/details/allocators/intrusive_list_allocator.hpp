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
class intrusive_list_allocator : private utils::ebo<UpstreamAlloc>,
                                 private utils::noncopyable, private utils::nonmovable
{
    struct control_block
    {
        control_block*   m_next;
        control_block*   m_prev;
        byte*            m_memblk;
        size_t           m_size;
    };
public:
    typedef typename Descriptor::pointer_type pointer_type;
    typedef stateful_alloc_tag alloc_tag;

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
            return proxy(get_descriptor(m_control_block)->get_mem());
        }
    private:
        friend class intrusive_list_allocator;
        friend class boost::iterator_core_access;

        iterator(control_block* cblk) noexcept
            : m_control_block(cblk)
        {}

        byte* memblk() const
        {
            return m_control_block->m_memblk;
        }

        control_block* cblk() const
        {
            return m_control_block;
        }

        Descriptor* descriptor() const
        {
            return get_descriptor(m_control_block);
        }

        pointer_type dereference() const
        {
            return get_descriptor(m_control_block)->get_mem();
        }

        void increment() noexcept
        {
            m_control_block = m_control_block->m_next;
        }

        void decrement() noexcept
        {
            m_control_block = m_control_block->m_prev;
        }

        bool equal(const iterator& other) const noexcept
        {
            return m_control_block == other.m_control_block;
        }

        control_block* m_control_block;
    };

    typedef utils::locked_range<
              boost::iterator_range<iterator>
            , Lock
        > memory_range_type;

    intrusive_list_allocator()
        : m_head(get_fake_block())
    {
        m_fake.m_next   = get_fake_block();
        m_fake.m_prev   = get_fake_block();
        m_fake.m_memblk = nullptr;
        m_fake.m_size   = 0;
    }

    ~intrusive_list_allocator()
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        for (iterator it = begin(); it != end(); )
        {
            iterator next = std::next(it, 1);
            destroy_memblk(it.cblk());
            it = next;
        }
    }

    pointer_type allocate(size_t size)
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        return create_memblk(size)->get_mem();
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        iterator it = begin();
        while (it != end() && *it != ptr) {
            ++it;
        }
        if (it != end()) {
            destroy_memblk(it.cblk());
        }
    }

    size_t shrink()
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        size_t freed = 0;
        for (auto it = begin(); it != end(); ) {
            auto next = std::next(it);
            if (it.descriptor()->empty()) {
                freed += it.cblk()->m_size;
                destroy_memblk(it.cblk());
            }
            it = next;
        }
        return freed;
    }

    template <typename Functor>
    void apply_to_descriptors(Functor&& f)
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        for (auto it = begin(); it != end(); ++it)
        {
            f(*it.descriptor());
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
    iterator begin()
    {
        return iterator(m_head);
    }

    iterator end()
    {
        return iterator(get_fake_block());
    }

    Descriptor* create_memblk(size_t size)
    {
        size_t memblk_size = size + sizeof(control_block) + sizeof(Descriptor);
        byte*  memblk = upstream_allocate(memblk_size);

        control_block* cblk = get_control_block(memblk, memblk_size);
        Descriptor* descriptor = get_descriptor(memblk, memblk_size);

        cblk->m_next    = m_head;
        cblk->m_prev    = get_fake_block();
        cblk->m_memblk  = memblk;
        cblk->m_size    = memblk_size;

        m_head->m_prev = cblk;
        get_fake_block()->m_next = cblk;

        m_head = cblk;

        new (descriptor) Descriptor(get_mem(memblk), size);

        return descriptor;
    }

    void destroy_memblk(control_block* cblk)
    {
        if (cblk == m_head) {
            m_head = cblk->m_next;
        }
        cblk->m_next->m_prev = cblk->m_prev;
        cblk->m_prev->m_next = cblk->m_next;

        get_descriptor(cblk)->~Descriptor();

        upstream_deallocate(cblk->m_memblk, cblk->m_size);
    }

    byte* upstream_allocate(size_t size)
    {
        return this->template get_base<UpstreamAlloc>().allocate(size);
    }

    void upstream_deallocate(byte* ptr, size_t size)
    {
        this->template get_base<UpstreamAlloc>().deallocate(ptr, size);
    }

    control_block* get_fake_block()
    {
        return &m_fake;
    }

    static control_block* get_control_block(byte* memblk, size_t memblk_size)
    {
        return reinterpret_cast<control_block*>(memblk + memblk_size - sizeof(Descriptor) - sizeof(control_block));
    }

    static Descriptor* get_descriptor(byte* memblk, size_t memblk_size)
    {
        return reinterpret_cast<Descriptor*>(memblk + memblk_size - sizeof(Descriptor));
    }

    static Descriptor* get_descriptor(control_block* cblk)
    {
        return reinterpret_cast<Descriptor*>(reinterpret_cast<byte*>(cblk) + sizeof(control_block));
    }

    static byte* get_mem(byte* memblk)
    {
        return memblk;
    }

    control_block m_fake;
    control_block* m_head;
    Lock m_lock;
};

}}}

#endif //DIPLOMA_FREELIST_ALLOCATOR_HPP
