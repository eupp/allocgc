#ifndef DIPLOMA_MSO_ALLOCATOR_HPP
#define DIPLOMA_MSO_ALLOCATOR_HPP

#include <cstring>
#include <list>

#include <libprecisegc/details/allocators/intrusive_list_allocator.hpp>
#include <libprecisegc/details/allocators/freelist_pool_chunk.hpp>
#include <libprecisegc/details/allocators/managed_pool_chunk.hpp>
#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/core_allocator.hpp>
#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/allocators/stl_adapter.hpp>

#include <libprecisegc/details/utils/flatten_range.hpp>
#include <libprecisegc/details/utils/locked_range.hpp>
#include <libprecisegc/details/utils/utility.hpp>

#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace allocators {

class mso_allocator : private utils::noncopyable, utils::nonmovable
{
    typedef managed_pool_chunk chunk_t;

    typedef allocators::intrusive_list_allocator<
            allocators::freelist_pool_chunk
            , allocators::default_allocator
            , allocators::single_chunk_with_search_cache
            , utils::dummy_mutex
    > chunk_pool_t;

    typedef std::list<chunk_t, stl_adapter<chunk_t, chunk_pool_t>> chunk_list_t;
    typedef typename chunk_list_t::iterator iterator_t;
public:
    typedef utils::flattened_range<iterator_t> memory_range_type;
    typedef gc_alloc_descriptor pointer_type;
    typedef stateful_alloc_tag alloc_tag;

    mso_allocator();

    pointer_type allocate(size_t size);

    void deallocate(pointer_type ptr, size_t size);

    heap_part_stat collect();

    size_t shrink();

    void sweep();

    memory_range_type memory_range()
    {
        return utils::flatten_range(m_chunks.begin(), m_chunks.end());
    }

    bool empty() const;

    double residency() const;
private:
    void sweep_chunk(iterator_t chk, byte* mem_start, byte* mem_end);

    iterator_t create_chunk(size_t cell_size);
    iterator_t destroy_chunk(typename chunk_list_t::iterator chk);

    std::pair<byte*, size_t> allocate_block(size_t cell_size);
    void deallocate_block(byte* ptr, size_t size);

    chunk_list_t m_chunks;
    byte* m_freelist;
    byte* m_top;
    byte* m_end;
};

}}}

#endif //DIPLOMA_MSO_ALLOCATOR_HPP
