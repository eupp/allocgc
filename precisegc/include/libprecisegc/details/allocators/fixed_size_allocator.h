#ifndef DIPLOMA_FIXED_SIZE_ALLOCATOR_H
#define DIPLOMA_FIXED_SIZE_ALLOCATOR_H

#include <cstddef>
#include <vector>
#include <algorithm>

#include "stl_adapter.h"
#include "joined_range.h"
#include "../util.h"

namespace precisegc { namespace details { namespace allocators {

template <typename Chunk, typename Alloc, typename InternalAlloc>
class fixed_size_allocator : private ebo<Alloc>, private noncopyable
{
    typedef std::vector<Chunk, stl_adapter<Chunk, InternalAlloc>> vector_t;
public:
    typedef typename Chunk::pointer_type pointer_type;
    typedef joined_range<vector_t> range_type;

    fixed_size_allocator()
    {
        m_alloc_chunk = m_chunks.begin();
    }

    fixed_size_allocator(fixed_size_allocator&&) = default;

    pointer_type allocate(size_t size)
    {
        if (m_alloc_chunk == m_chunks.end()) {
            m_chunks.push_back(Chunk::create(size, get_allocator()));
            m_alloc_chunk = std::prev(m_chunks.end(), 1);
            return m_alloc_chunk->allocate(size);
        }
        if (m_alloc_chunk->memory_available()) {
            return m_alloc_chunk->allocate(size);
        }

        m_alloc_chunk = std::find_if(m_alloc_chunk, m_chunks.end(),
                                     [] (const Chunk& chk) { return chk.memory_available(); });
        return allocate(size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        using std::swap;

        auto dealloc_chunk = std::find_if(m_chunks.begin(), m_chunks.end(),
                                          [&ptr] (const Chunk& chk) { return chk.contains(ptr); });
        if (dealloc_chunk != m_chunks.end()) {
            dealloc_chunk->deallocate(ptr, size);
            if (dealloc_chunk->empty(size)) {
                ptrdiff_t alloc_chunk_ind = m_alloc_chunk - m_chunks.begin();

                Chunk::destroy(*dealloc_chunk, size, get_allocator());
                auto last = std::prev(m_chunks.end(), 1);
                swap(*dealloc_chunk, *last);
                m_chunks.pop_back();

                m_alloc_chunk = std::next(m_chunks.begin(), alloc_chunk_ind);
            }
        }
    }

    void reset_cache()
    {
        m_alloc_chunk = m_chunks.begin();
    }

    range_type range()
    {
        return range_type(m_chunks);
    }

    const Alloc& get_allocator() const
    {
        return this->template get_base<Alloc>();
    }

    const Alloc& get_const_allocator() const
    {
        return this->template get_base<Alloc>();
    }

private:
    Alloc& get_allocator()
    {
        return this->template get_base<Alloc>();
    }

    typename vector_t::iterator m_alloc_chunk;
    vector_t m_chunks;
};

}}}

#endif //DIPLOMA_FIXED_SIZE_ALLOCATOR_H
