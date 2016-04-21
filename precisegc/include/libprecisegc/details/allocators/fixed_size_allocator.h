#ifndef DIPLOMA_FIXED_SIZE_ALLOCATOR_H
#define DIPLOMA_FIXED_SIZE_ALLOCATOR_H

#include <cstddef>
#include <vector>
#include <algorithm>
#include <memory>
#include <list>

#include "stl_adapter.h"
#include "joined_range.h"
#include "constants.h"
#include "../util.h"

namespace precisegc { namespace details { namespace allocators {

template <typename Chunk, typename Alloc, typename InternalAlloc>
class fixed_size_allocator : private ebo<Alloc>, private noncopyable
{
    typedef std::list<Chunk, std::allocator<Chunk>> list_t;
public:
    typedef typename Chunk::pointer_type pointer_type;
    typedef joined_range<list_t> range_type;

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
        auto dealloc_chunk = std::find_if(m_chunks.begin(), m_chunks.end(),
                                          [&ptr] (const Chunk& chk) { return chk.contains(ptr); });
        if (dealloc_chunk != m_chunks.end()) {
            dealloc_chunk->deallocate(ptr, size);
            if (dealloc_chunk->empty(size)) {
                if (m_alloc_chunk == dealloc_chunk) {
                    m_alloc_chunk++;
                }
                destroy_chunk(dealloc_chunk, size);
            }
        }
    }

    size_t shrink()
    {
        size_t size = 0;
        for (auto it = m_chunks.begin(), end = m_chunks.end(); it != end; ) {
            if (it->is_dead()) {
                size += it->get_mem_size();
                it = destroy_chunk(it, it->get_cell_size());
            } else {
//                it->reset_bits();
                ++it;
            }
        }
        reset_cache();
        return size;
    }

    void reset_bits()
    {
        for (auto it = m_chunks.begin(), end = m_chunks.end(); it != end; ++it) {
            it->reset_bits();
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
    typename list_t::iterator destroy_chunk(typename list_t::iterator chk, size_t cell_size)
    {
        Chunk::destroy(*chk, cell_size, get_allocator());
        return m_chunks.erase(chk);
    }

    Alloc& get_allocator()
    {
        return this->template get_base<Alloc>();
    }

    typename list_t::iterator m_alloc_chunk;
    list_t m_chunks;
};

}}}

#endif //DIPLOMA_FIXED_SIZE_ALLOCATOR_H
