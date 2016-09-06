#ifndef DIPLOMA_FREELIST_ALLOCATOR_HPP
#define DIPLOMA_FREELIST_ALLOCATOR_HPP

#include <cstddef>
#include <type_traits>

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/utils/flatten_range.hpp>
#include <libprecisegc/details/utils/locked_range.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace allocators {

template <typename Chunk,
          typename UpstreamAlloc,
          template <typename> class CachePolicy,
          typename Lock>
class intrusive_list_allocator : private utils::ebo<UpstreamAlloc>,
                                 private utils::noncopyable, private utils::nonmovable
{
public:
    typedef typename Chunk::pointer_type pointer_type;
private:
    struct control_block
    {
        control_block*   m_next;
        control_block*   m_prev;
    };

    class chunk_iterator: public boost::iterator_facade<
              chunk_iterator
            , Chunk
            , boost::bidirectional_traversal_tag
        >
    {
    public:
        chunk_iterator(const chunk_iterator&) noexcept = default;
        chunk_iterator(chunk_iterator&&) noexcept = default;

        chunk_iterator& operator=(const chunk_iterator&) noexcept = default;
        chunk_iterator& operator=(chunk_iterator&&) noexcept = default;

        Chunk* operator->() const
        {
            return get_chunk(m_control_block);
        }
    private:
        friend class intrusive_list_allocator;
        friend class boost::iterator_core_access;

        chunk_iterator(control_block* cblk) noexcept
            : m_control_block(cblk)
        {}

        byte* memblk() const
        {
            return get_chunk(m_control_block)->get_mem();
        }

        control_block* cblk() const
        {
            return m_control_block;
        }

        Chunk* chunk() const
        {
            return get_chunk(m_control_block);
        }

        Chunk& dereference() const
        {
            return *get_chunk(m_control_block);
        }

        void increment() noexcept
        {
            m_control_block = m_control_block->m_next;
        }

        void decrement() noexcept
        {
            m_control_block = m_control_block->m_prev;
        }

        bool equal(const chunk_iterator& other) const noexcept
        {
            return m_control_block == other.m_control_block;
        }

        control_block* m_control_block;
    };

    class mem_iterator : public boost::iterator_adaptor<
              mem_iterator
            , chunk_iterator
            , typename std::iterator_traits<typename Chunk::iterator>::value_type
            , boost::bidirectional_traversal_tag
            , typename std::iterator_traits<typename Chunk::iterator>::value_type
         >
    {
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
    public:
//        typedef typename mem_iterator::iterator_adaptor_::value_type       value_type;
        typedef typename mem_iterator::iterator_adaptor_::reference        reference;
        typedef typename mem_iterator::iterator_adaptor_::pointer          pointer;
        typedef typename mem_iterator::iterator_adaptor_::difference_type  difference_type;

        mem_iterator(const mem_iterator&) noexcept = default;
        mem_iterator(mem_iterator&&) noexcept = default;

        mem_iterator& operator=(const mem_iterator&) noexcept = default;
        mem_iterator& operator=(mem_iterator&&) noexcept = default;

        proxy operator->() const
        {
            return proxy(dereference());
        }
    private:
        friend class intrusive_list_allocator;
        friend class boost::iterator_core_access;

        mem_iterator(control_block* cblk) noexcept
            : mem_iterator::iterator_adaptor_(cblk)
        {}

        reference dereference() const
        {
            return *(*this->base_reference()).begin();
        }
    };

    typedef typename std::conditional<
              is_multi_block_chunk<Chunk>::value
            , utils::flattened_range<boost::iterator_range<chunk_iterator>>
            , boost::iterator_range<mem_iterator>
        >::type memory_range_unlocked_type;

    typedef CachePolicy<chunk_iterator> cache_t;
public:
    typedef stateful_alloc_tag alloc_tag;
    typedef utils::locked_range<
              memory_range_unlocked_type
            , Lock
        > memory_range_type;

    intrusive_list_allocator()
        : m_head(get_fake_block())
        , m_cache(begin())
    {
        m_fake.m_next = get_fake_block();
        m_fake.m_prev = get_fake_block();
    }

    ~intrusive_list_allocator()
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        for (auto it = begin(); it != end(); ) {
            auto next = std::next(it);
            destroy_memblk(it.cblk());
            it = next;
        }
    }

    pointer_type allocate(size_t size)
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        if (!m_cache.memory_available(begin(), end())) {
            auto new_chunk = create_memblk(size);
            m_cache.update(new_chunk);
            return new_chunk->allocate(size);
        }
        return m_cache.allocate(size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        auto dealloc_chunk = std::find_if(begin(), end(),
                                          [&ptr] (const Chunk& chk) { return chk.contains(ptr); });
        dealloc_chunk->deallocate(ptr, size);
        if (dealloc_chunk->empty()) {
            destroy_memblk(dealloc_chunk);
        }
    }

    size_t shrink()
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        size_t freed = 0;
        for (auto it = begin(); it != end(); ) {
            auto next = std::next(it);
            if (it.chunk()->empty()) {
                freed += it.chunk()->get_mem_size();
                destroy_memblk(it.cblk());
            }
            it = next;
        }
        return freed;
    }

    template <typename Functor>
    void apply_to_chunks(Functor&& f)
    {
        std::lock_guard<Lock> lock_guard(m_lock);
        for (auto it = begin(); it != end(); ++it) {
            f(*it);
        }
    }

    memory_range_type memory_range()
    {
        std::unique_lock<Lock> lock_guard(m_lock);
        return memory_range_type(memory_range_unlocked(), std::move(lock_guard));
    }

    const UpstreamAlloc& upstream_allocator() const
    {
        return this->template get_base<UpstreamAlloc>();
    }
private:
    template <typename C = Chunk>
    auto memory_range_unlocked()
        -> typename std::enable_if<is_multi_block_chunk<C>::value, memory_range_unlocked_type>::type
    {
        auto rng = boost::make_iterator_range(begin(), end());
        return utils::flatten_range(rng);
    }

    template <typename C = Chunk>
    auto memory_range_unlocked()
        -> typename std::enable_if<!is_multi_block_chunk<C>::value, memory_range_unlocked_type>::type
    {
        return boost::make_iterator_range(mem_iterator(m_head), mem_iterator(get_fake_block()));
    }

    chunk_iterator begin()
    {
        return chunk_iterator(m_head);
    }

    chunk_iterator end()
    {
        return chunk_iterator(get_fake_block());
    }

    chunk_iterator create_memblk(size_t cell_size)
    {
        size_t chunk_size = Chunk::chunk_size(cell_size);
        size_t memblk_size = chunk_size + sizeof(control_block) + sizeof(Chunk);

        auto deleter = [this, memblk_size] (byte* p) {
            upstream_deallocate(p, memblk_size);
        };
        std::unique_ptr<byte, decltype(deleter)> memblk_owner(upstream_allocate(memblk_size), deleter);
        byte*  memblk = memblk_owner.get();

        control_block* fake = get_fake_block();
        control_block* cblk = get_control_block(memblk, memblk_size);
        Chunk* chunk = get_chunk(memblk, memblk_size);
        new (chunk) Chunk(get_mem(memblk), chunk_size, cell_size);

        cblk->m_next = fake;
        cblk->m_prev = fake->m_prev;

        fake->m_prev->m_next = cblk;
        fake->m_prev = cblk;

        if (m_head == fake) {
            m_head = cblk;
        }
        memblk_owner.release();

        return chunk_iterator(cblk);
    }

    void destroy_memblk(chunk_iterator it)
    {
        m_cache.invalidate(it, end());

        control_block* cblk = it.cblk();
        if (cblk == m_head) {
            m_head = cblk->m_next;
        }
        cblk->m_next->m_prev = cblk->m_prev;
        cblk->m_prev->m_next = cblk->m_next;

        Chunk* chunk = get_chunk(cblk);
        get_chunk(cblk)->~Chunk();

        upstream_deallocate(chunk->get_mem(),
                            chunk->get_mem_size() + sizeof(control_block) + sizeof(Chunk));
    }

    byte* upstream_allocate(size_t size)
    {
        return this->template get_base<UpstreamAlloc>().allocate(size, PAGE_SIZE);
    }

    void upstream_deallocate(byte* ptr, size_t size)
    {
        this->template get_base<UpstreamAlloc>().deallocate(ptr, size, PAGE_SIZE);
    }

    control_block* get_fake_block()
    {
        return &m_fake;
    }

    static control_block* get_control_block(byte* memblk, size_t memblk_size)
    {
        assert(memblk && memblk_size > 0);
        return reinterpret_cast<control_block*>(memblk + memblk_size - sizeof(Chunk) - sizeof(control_block));
    }

    static Chunk* get_chunk(byte* memblk, size_t memblk_size)
    {
        assert(memblk && memblk_size > 0);
        return reinterpret_cast<Chunk*>(memblk + memblk_size - sizeof(Chunk));
    }

    static Chunk* get_chunk(control_block* cblk)
    {
        assert(cblk);
        return reinterpret_cast<Chunk*>(reinterpret_cast<byte*>(cblk) + sizeof(control_block));
    }

    static byte* get_mem(byte* memblk)
    {
        return memblk;
    }

    control_block m_fake;
    control_block* m_head;
    cache_t m_cache;
    Lock m_lock;
};

}}}

#endif //DIPLOMA_FREELIST_ALLOCATOR_HPP
