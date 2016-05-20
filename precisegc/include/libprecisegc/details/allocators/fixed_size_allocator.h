#ifndef DIPLOMA_FIXED_SIZE_ALLOCATOR_H
#define DIPLOMA_FIXED_SIZE_ALLOCATOR_H

#include <cstddef>
#include <vector>
#include <algorithm>
#include <memory>
#include <list>

#include <boost/range/iterator_range.hpp>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/allocators/stl_adapter.h>
#include <libprecisegc/details/utils/flatten_range.hpp>
#include <libprecisegc/details/util.h>
#include <libprecisegc/details/constants.h>

namespace precisegc { namespace details { namespace allocators {

template <typename Chunk, typename UpstreamAlloc, typename InternalAlloc>
class fixed_size_allocator : private ebo<UpstreamAlloc>, private noncopyable
{
    typedef std::list<Chunk, stl_adapter<Chunk, InternalAlloc>> list_t;
public:
    typedef typename Chunk::pointer_type pointer_type;
    typedef stateful_alloc_tag alloc_tag;
    typedef utils::flattened_range<boost::iterator_range<typename list_t::iterator>> memory_range_type;

    fixed_size_allocator()
    {
        m_alloc_chunk = m_chunks.begin();
    }

    fixed_size_allocator(fixed_size_allocator&&) = default;

    pointer_type allocate(size_t size)
    {
        if (m_alloc_chunk == m_chunks.end()) {
            m_alloc_chunk = create_chunk(size);
            return m_alloc_chunk->allocate(size);
        }
        if (m_alloc_chunk->memory_available()) {
            return m_alloc_chunk->allocate(size);
        }

        m_alloc_chunk = std::find_if(++m_alloc_chunk, m_chunks.end(),
                                     [] (const Chunk& chk) { return chk.memory_available(); });
        return allocate(size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        auto dealloc_chunk = std::find_if(m_chunks.begin(), m_chunks.end(),
                                          [&ptr] (const Chunk& chk) { return chk.contains(ptr); });
        if (dealloc_chunk != m_chunks.end()) {
            dealloc_chunk->deallocate(ptr, size);
            if (dealloc_chunk->empty(size)) {
                destroy_chunk(dealloc_chunk);
            }
        }
    }

    size_t shrink(size_t cell_size)
    {
        size_t shrunk = 0;
        for (auto it = m_chunks.begin(), end = m_chunks.end(); it != end; ) {
            if (it->empty(cell_size)) {
                shrunk += it->get_mem_size();
                it = destroy_chunk(it);
            } else {
                ++it;
            }
        }
        return shrunk;
    }

    void reset_cache()
    {
        m_alloc_chunk = m_chunks.begin();
    }

    memory_range_type memory_range()
    {
        return utils::flatten_range(m_chunks);
    }

    const UpstreamAlloc& get_allocator() const
    {
        return this->template get_base<UpstreamAlloc>();
    }

    const UpstreamAlloc& get_const_allocator() const
    {
        return this->template get_base<UpstreamAlloc>();
    }

private:
    typedef typename UpstreamAlloc::pointer_type internal_pointer_type;

    std::pair<internal_pointer_type, size_t> allocate_block(size_t cell_size)
    {
        assert(PAGE_SIZE % cell_size == 0);
        size_t chunk_cnt = std::max((size_t) Chunk::CHUNK_MINSIZE, PAGE_SIZE / cell_size);
        chunk_cnt = std::min((size_t) Chunk::CHUNK_MAXSIZE, chunk_cnt);
        assert(chunk_cnt <= Chunk::CHUNK_MAXSIZE);
        size_t chunk_size = chunk_cnt * cell_size;
        assert(chunk_size <= PAGE_SIZE);
        return std::make_pair(get_allocator().allocate(chunk_size), chunk_size);
    }

    void deallocate_block(internal_pointer_type p, size_t size)
    {
        assert(p);
        get_allocator().deallocate(p, size);
    }

    typename list_t::iterator create_chunk(size_t cell_size)
    {
        auto alloc_res = allocate_block(cell_size);
        m_chunks.emplace_back(alloc_res.first, alloc_res.second, cell_size);
        return --m_chunks.end();
    }
    
    typename list_t::iterator destroy_chunk(typename list_t::iterator chk)
    {
        if (m_alloc_chunk == chk) {
            m_alloc_chunk++;
        }
        deallocate_block(chk->get_mem(), chk->get_mem_size());
        return m_chunks.erase(chk);
    }

    UpstreamAlloc& get_allocator()
    {
        return this->template get_base<UpstreamAlloc>();
    }

    typename list_t::iterator m_alloc_chunk;
    list_t m_chunks;
};

}}}

#endif //DIPLOMA_FIXED_SIZE_ALLOCATOR_H
