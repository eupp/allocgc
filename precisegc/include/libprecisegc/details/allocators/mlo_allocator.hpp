#ifndef DIPLOMA_MLO_ALLOCATOR_HPP
#define DIPLOMA_MLO_ALLOCATOR_HPP

#include <mutex>

#include <boost/range/iterator_range.hpp>

#include <libprecisegc/details/allocators/managed_object_descriptor.hpp>
#include <libprecisegc/details/allocators/managed_memory_iterator.hpp>
#include <libprecisegc/details/allocators/allocator_tag.hpp>

#include <libprecisegc/details/utils/flatten_range.hpp>
#include <libprecisegc/details/utils/locked_range.hpp>
#include <libprecisegc/details/utils/utility.hpp>

#include <libprecisegc/details/compacting/forwarding.hpp>

#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/gc_alloc_messaging.hpp>

namespace precisegc { namespace details { namespace allocators {

class mlo_allocator : private utils::noncopyable, private utils::nonmovable
{
    struct control_block
    {
        control_block*   m_next;
        control_block*   m_prev;
    };

    typedef managed_object_descriptor descriptor_t;

    class iterator: public boost::iterator_facade<
              iterator
            , descriptor_t
            , boost::bidirectional_traversal_tag
        >
    {
    public:
        iterator(const iterator&) noexcept = default;
        iterator(iterator&&) noexcept = default;

        iterator& operator=(const iterator&) noexcept = default;
        iterator& operator=(iterator&&) noexcept = default;

        descriptor_t* operator->() const;
    private:
        friend class mlo_allocator;
        friend class boost::iterator_core_access;

        iterator(control_block* cblk) noexcept;

        byte* memblk() const;

        descriptor_t* get() const;

        descriptor_t& dereference() const;
        bool equal(const iterator& other) const noexcept;
        void increment() noexcept;
        void decrement() noexcept;

        control_block* m_control_block;
    };

    class memory_iterator:
              public managed_memory_iterator<descriptor_t>
            , public boost::iterator_facade<
                  memory_iterator
                , managed_memory_iterator<descriptor_t>::proxy_t
                , boost::random_access_traversal_tag
                , managed_memory_iterator<descriptor_t>::proxy_t
            >
    {
    public:
        memory_iterator();
        memory_iterator(byte* ptr, descriptor_t* descr, control_block* cblk);

        const proxy_t* operator->()
        {
            return &m_proxy;
        }
    private:
        friend class boost::iterator_core_access;

        void increment() noexcept;
        void decrement() noexcept;

        bool equal(const memory_iterator& other) const noexcept;

        control_block* get_cblk();

        control_block* m_cblk;
    };

    typedef boost::iterator_range<memory_iterator> memory_range_type;
public:
    typedef gc_alloc_response pointer_type;
    typedef stateful_alloc_tag alloc_tag;

    mlo_allocator();
    ~mlo_allocator();

    gc_alloc_response allocate(const gc_alloc_request& rqst);

    gc_heap_stat collect(compacting::forwarding& frwd);
    void fix(const compacting::forwarding& frwd);

    // temporary until refactor
    memory_range_type memory_range();
private:
    typedef std::mutex  mutex_t;

    static descriptor_t*  get_descriptor(byte* memblk);
    static control_block* get_control_block(byte* memblk);
    static byte*          get_mem(byte* memblk);

    void destroy(byte* ptr);

    iterator begin();
    iterator end();

    control_block* get_fake_block();

    byte* allocate_block(size_t size);
    void  deallocate_block(byte* ptr, size_t size);

    control_block  m_fake;
    control_block* m_head;
    mutex_t m_mutex;
};

}}}

#endif //DIPLOMA_MLO_ALLOCATOR_HPP
