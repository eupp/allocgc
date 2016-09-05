#ifndef DIPLOMA_INTRUSIVE_intrusive_list_allocator_HPP
#define DIPLOMA_INTRUSIVE_intrusive_list_allocator_HPP

#include <cassert>
#include <cstring>
#include <memory>
#include <algorithm>
#include <type_traits>

#include <boost/iterator/iterator_facade.hpp>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace allocators {

namespace internals {
struct control_block
{
    byte*   m_prev;
    byte*   m_next;
};
}

template <typename Chunk>
constexpr size_t block_size_for(size_t cell_size, size_t cnt)
{
    return cell_size * cnt + sizeof(internals::control_block) + sizeof(Chunk);
}

template <typename Chunk, typename UpstreamAlloc, size_t BlockSize = 4096>
class intrusive_list_pool_allocator : private utils::ebo<UpstreamAlloc>, private utils::noncopyable, private utils::nonmovable
{
public:
    static_assert(std::is_same<byte*, typename Chunk::pointer_type>::value,
                  "intrusive_list_pool_allocator works only with raw pointers");

    typedef byte* pointer_type;
    typedef stateful_alloc_tag alloc_tag;

    intrusive_list_pool_allocator()
            : m_head(get_fake_block())
            , m_alloc_chunk(m_head)
            , m_freelist(nullptr)
    {
        m_fake.m_next = get_fake_block();
        m_fake.m_prev = get_fake_block();
    }

    ~intrusive_list_pool_allocator()
    {
        for (iterator it = begin(), last = end(); it != last; ) {
            it = destroy_chunk(it);
        }
    }

    pointer_type allocate(size_t size)
    {
        if (m_freelist) {
            byte* p = m_freelist;
            m_freelist = *reinterpret_cast<byte**>(m_freelist);
            return p;
        }
        if (m_alloc_chunk == end()) {
            m_alloc_chunk = create_chunk(size);
            return m_alloc_chunk->allocate(size);
        }
        if (m_alloc_chunk->memory_available()) {
            return m_alloc_chunk->allocate(size);
        }

        auto new_alloc_chunk = std::find_if(std::next(m_alloc_chunk), end(),
                                            [] (const Chunk& chk) { return chk.memory_available(); });
        if (new_alloc_chunk == end()) {
            new_alloc_chunk = std::find_if(begin(), m_alloc_chunk,
                                           [] (const Chunk& chk) { return chk.memory_available(); });
            if (new_alloc_chunk == m_alloc_chunk) {
                m_alloc_chunk = create_chunk(size);
            } else {
                m_alloc_chunk = new_alloc_chunk;
            }
        } else {
            m_alloc_chunk = new_alloc_chunk;
        }

        return m_alloc_chunk->allocate(size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        assert(ptr /*&& reinterpret_cast<std::uintptr_t>(ptr) % size == 0*/);
        memcpy(ptr, &m_freelist, sizeof(byte*));
        m_freelist = ptr;
    }

    size_t shrink(size_t cell_size)
    {
        size_t shrunk = 0;
        while (m_freelist) {
            byte* next = *reinterpret_cast<byte**>(m_freelist);
            shrunk += deallocate_from_chunk(m_freelist, cell_size);
            m_freelist = next;
        }
        return shrunk;
    }

    template <typename Functor>
    void apply_to_chunks(Functor&& f)
    {
        std::for_each(begin(), end(), f);
    }

    const UpstreamAlloc& upstream_allocator() const
    {
        return this->template get_base<UpstreamAlloc>();
    }
private:
    class iterator: public boost::iterator_facade<
            iterator
            , Chunk
            , boost::bidirectional_traversal_tag
    >
    {
    public:
        iterator(byte* memblk) noexcept
                : m_memblk(memblk)
        {}

        iterator(const iterator&) noexcept = default;
        iterator(iterator&&) noexcept = default;

        iterator& operator=(const iterator&) noexcept = default;
        iterator& operator=(iterator&&) noexcept = default;

        Chunk* operator->() const
        {
            return &get_chunk(m_memblk);
        }
    private:
        friend class intrusive_list_pool_allocator;
        friend class boost::iterator_core_access;

        Chunk& dereference() const
        {
            return get_chunk(m_memblk);
        }

        void increment() noexcept
        {
            m_memblk = get_control_block(m_memblk).m_next;
        }

        void decrement() noexcept
        {
            m_memblk = get_control_block(m_memblk).m_prev;
        }

        bool equal(const iterator& other) const noexcept
        {
            return m_memblk == other.m_memblk;
        }

        byte* m_memblk;
    };

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

    iterator create_chunk(size_t cell_size)
    {
        auto deleter = [this] (byte* p) {
            upstream_deallocate(p, BlockSize);
        };
        std::unique_ptr<byte, decltype(deleter)> memblk_owner(upstream_allocate(BlockSize), deleter);
        byte* memblk = memblk_owner.get();

        size_t free_mem_in_block = BlockSize - (sizeof(internals::control_block) + sizeof(Chunk));
        size_t mem_available = std::min((free_mem_in_block / cell_size) * cell_size, get_chunk_mem_max_size(cell_size));
        new (&get_chunk(memblk)) Chunk(get_mem(memblk), mem_available, cell_size);

        get_control_block(memblk).m_next = m_head;
        get_control_block(memblk).m_prev = get_fake_block();

        get_control_block(m_head).m_prev = memblk;
        get_control_block(get_fake_block()).m_next = memblk;

        m_head = memblk;
        memblk_owner.release();

        return iterator(memblk);
    }

    iterator destroy_chunk(iterator chk)
    {
        assert(chk != end());
        if (m_alloc_chunk == chk) {
            ++m_alloc_chunk;
        }
        iterator next = std::next(chk);
        iterator prev = std::prev(chk);
        get_control_block(prev.m_memblk).m_next = next.m_memblk;
        get_control_block(next.m_memblk).m_prev = prev.m_memblk;
        if (chk == begin()) {
            m_head = next.m_memblk;
        }
        upstream_deallocate(chk.m_memblk, BlockSize);
        return next;
    }

    size_t deallocate_from_chunk(byte* ptr, size_t size)
    {
        auto dealloc_chunk = std::find_if(begin(), end(),
                                          [&ptr] (const Chunk& chk) { return chk.contains(ptr); });
        if (dealloc_chunk != end()) {
            dealloc_chunk->deallocate(ptr, size);
            if (dealloc_chunk->empty()) {
                size_t chunk_size = sizeof(internals::control_block) + sizeof(Chunk) + dealloc_chunk->get_mem_size();
                destroy_chunk(dealloc_chunk);
                return chunk_size;
            }
        }
        return 0;
    }

    byte* upstream_allocate(size_t size)
    {
        return this->template get_base<UpstreamAlloc>().allocate(size);
    }

    void upstream_deallocate(byte* ptr, size_t size)
    {
        this->template get_base<UpstreamAlloc>().deallocate(ptr, size);
    }

    static internals::control_block& get_control_block(byte* memblk)
    {
        return *reinterpret_cast<internals::control_block*>(memblk);
    }

    static Chunk& get_chunk(byte* memblk)
    {
        return *reinterpret_cast<Chunk*>(memblk + sizeof(internals::control_block));
    }

    static byte* get_mem(byte* memblk)
    {
        return memblk + sizeof(internals::control_block) + sizeof(Chunk);
    }

    static size_t get_chunk_mem_max_size(size_t cell_size)
    {
        if (Chunk::CHUNK_MAXSIZE == std::numeric_limits<size_t>::max()) {
            return std::numeric_limits<size_t>::max();
        }
        return Chunk::CHUNK_MAXSIZE * cell_size;
    }

    static size_t get_chunk_block_max_size(size_t cell_size)
    {
        if (Chunk::CHUNK_MAXSIZE == std::numeric_limits<size_t>::max()) {
            return std::numeric_limits<size_t>::max();
        }
        return Chunk::CHUNK_MAXSIZE * cell_size + sizeof(internals::control_block) + sizeof(Chunk);
    }

    internals::control_block m_fake;
    byte* m_head;
    iterator m_alloc_chunk;
    byte* m_freelist;
};

}}}

#endif //DIPLOMA_INTRUSIVE_intrusive_list_allocator_HPP
