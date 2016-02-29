#ifndef DIPLOMA_FIXED_SIZE_ALLOCATOR_H
#define DIPLOMA_FIXED_SIZE_ALLOCATOR_H

#include <cstddef>
#include <vector>
#include <algorithm>

#include "stl_adapter.h"

namespace precisegc { namespace details { namespace allocators {

template <typename chunk, typename Alloc, typename InternalAlloc>
class fixed_size_allocator
{
public:
    typedef typename chunk::pointer_type pointer_type;

    fixed_size_allocator()
        : m_alloc_chunk(m_chunks.end())
    {}

    pointer_type allocate(size_t size)
    {
        if (m_alloc_chunk == m_chunks.end()) {
            m_chunks.push_back(chunk::create(m_allocator));
            m_alloc_chunk = std::prev(m_chunks.end(), 1);
            return m_alloc_chunk->allocate(size);
        }
        if (m_alloc_chunk->memory_available()) {
            return m_alloc_chunk->allocate(size);
        }

        m_alloc_chunk = std::find_if(m_chunks.begin(), m_chunks.end(),
                                     [] (const chunk& chk) { return chk.memory_available(); });
        return allocate(size);
    }

    void deallocate(pointer_type ptr, size_t size)
    {
        using std::swap;

        auto dealloc_chunk = std::find_if(m_chunks.begin(), m_chunks.end(),
                                          [ptr] (const chunk& chk) { return chk.contains(ptr); });
        if (dealloc_chunk != m_chunks.end()) {
            dealloc_chunk->deallocate(ptr, size);
            if (dealloc_chunk->empty(size)) {
                ptrdiff_t alloc_chunk_ind = m_alloc_chunk - m_chunks.begin();

                chunk::destroy(*dealloc_chunk, m_allocator);
                auto last = std::prev(m_chunks.end(), 1);
                swap(*dealloc_chunk, *last);
                m_chunks.pop_back();

                m_alloc_chunk = std::next(m_chunks.begin(), alloc_chunk_ind);
            }
        }
    }

    const Alloc& get_allocator() const
    {
        return m_allocator;
    }
private:
    typedef std::vector<chunk, stl_adapter<chunk, InternalAlloc>> vector_t;

    vector_t m_chunks;
    typename vector_t::iterator m_alloc_chunk;
    Alloc m_allocator;
};

}}}

#endif //DIPLOMA_FIXED_SIZE_ALLOCATOR_H
