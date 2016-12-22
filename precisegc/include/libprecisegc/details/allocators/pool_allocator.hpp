#ifndef DIPLOMA_POOL_ALLOCATOR_HPP
#define DIPLOMA_POOL_ALLOCATOR_HPP

#include <boost/iterator/iterator_adaptor.hpp>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/list_allocator.hpp>
#include <libprecisegc/details/allocators/freelist_pool_chunk.hpp>
#include <libprecisegc/details/utils/flatten_range.hpp>
#include <libprecisegc/details/utils/utility.hpp>


namespace precisegc { namespace details { namespace allocators {

template <typename UpstreamAlloc>
class pool_allocator : private utils::noncopyable, private utils::nonmovable
{
    typedef list_allocator<UpstreamAlloc> list_alloc_t;
    typedef freelist_pool_chunk chunk_t;

    class chunk_iterator : public boost::iterator_adaptor<
              chunk_iterator
            , typename list_alloc_t::iterator
            , chunk_t
            , boost::bidirectional_traversal_tag
        >
    {
    public:
        chunk_iterator(const chunk_iterator&) noexcept = default;
        chunk_iterator(chunk_iterator&&) noexcept = default;

        chunk_iterator& operator=(const chunk_iterator&) noexcept = default;
        chunk_iterator& operator=(chunk_iterator&&) noexcept = default;
    private:
        friend class pool_allocator;
        friend class boost::iterator_core_access;

        chunk_iterator(const typename list_alloc_t::iterator& it)
            : chunk_iterator::iterator_adaptor_(it)
        {}

        chunk_t& dereference() const
        {
            return *get_chk(*this->base());
        }
    };
public:
    typedef byte* pointer_type;
    typedef stateful_alloc_tag alloc_tag;

//    typedef utils::flattened_range<chunk_iterator> memory_range_type;

    pool_allocator()
        : m_freelist(nullptr)
    {}

    ~pool_allocator()
    {
        for (auto it = chunks_begin(); it != chunks_end(); ) {
            it = destroy_chunk(it);
        }
    }

    byte* allocate(size_t size)
    {
        if (m_freelist) {
            byte* ptr = m_freelist;
            m_freelist = *reinterpret_cast<byte**>(m_freelist);
            return ptr;
        }

        size_t memblk_size = get_memblk_size(size);
        size_t blk_size = get_blk_size(memblk_size);
        auto deleter = [this, blk_size] (byte* p) {
            upstream_deallocate(p, blk_size);
        };
        std::unique_ptr<byte, decltype(deleter)> blk(upstream_allocate(blk_size), deleter);

        chunk_t* chk = get_chk(blk.get());
        new (chk) chunk_t(get_memblk(blk.get()), memblk_size, size);

        blk.release();
        return chk->allocate(size);
    }

    void deallocate(byte* ptr, size_t size)
    {
        assert(ptr);
        byte** next = reinterpret_cast<byte**>(ptr);
        *next = m_freelist;
        m_freelist = ptr;
    }

    void shrink(size_t size)
    {
        while (m_freelist) {
            byte* next = *reinterpret_cast<byte**>(m_freelist);
            auto dealloc_chunk = std::find_if(chunks_begin(), chunks_end(),
                                              [this] (const chunk_t& chk) { return chk.contains(m_freelist); });
            dealloc_chunk->deallocate(m_freelist, size);
            m_freelist = next;
        }

        for (auto it = chunks_begin(); it != chunks_end(); ) {
            if (it->empty()) {
                it = destroy_chunk(it);
            } else {
                it = std::next(it);
            }
        }
    }

//    memory_range_type memory_range()
//    {
//        return utils::flatten_range(chunks_begin(), chunks_end());
//    }

    const UpstreamAlloc& upstream_allocator() const
    {
        return m_alloc.upstream_allocator();
    }
private:
    static constexpr size_t ALLOC_CNT = 32;

    static constexpr size_t get_memblk_size(size_t size)
    {
        return ALLOC_CNT * size;
    }

    static constexpr size_t get_blk_size(size_t size)
    {
        return size + sizeof(chunk_t);
    }

    static constexpr chunk_t* get_chk(byte* ptr)
    {
        return reinterpret_cast<chunk_t*>(ptr);
    }

    static constexpr chunk_t* get_chk_by_memblk(byte* ptr)
    {
        return reinterpret_cast<chunk_t*>(ptr - sizeof(chunk_t));
    }

    static constexpr byte* get_blk_by_chk(chunk_t* chk)
    {
        return reinterpret_cast<byte*>(chk);
    }

    static constexpr byte* get_memblk(byte* ptr)
    {
        return ptr + sizeof(chunk_t);
    }

    chunk_iterator destroy_chunk(const chunk_iterator& it)
    {
        chunk_iterator next = std::next(it);

        chunk_t* chk = &(*it);
        byte*    blk = get_blk_by_chk(chk);

        size_t memblk_size = chk->size();
        chk->~chunk_t();
        upstream_deallocate(blk, get_blk_size(memblk_size));

        return next;
    }

    chunk_iterator chunks_begin()
    {
        return chunk_iterator(m_alloc.begin());
    }

    chunk_iterator chunks_end()
    {
        return chunk_iterator(m_alloc.end());
    }

    byte* upstream_allocate(size_t size)
    {
        return m_alloc.allocate(size);
    }

    void upstream_deallocate(byte* ptr, size_t size)
    {
        m_alloc.deallocate(ptr, size);
    }

    list_alloc_t m_alloc;
    byte* m_freelist;
};

}}}

#endif //DIPLOMA_POOL_ALLOCATOR_HPP
