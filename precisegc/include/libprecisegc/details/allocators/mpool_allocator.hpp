#ifndef DIPLOMA_MPOOL_ALLOCATOR_HPP
#define DIPLOMA_MPOOL_ALLOCATOR_HPP

#include <list>
#include <cstring>
#include <utility>

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

#include <libprecisegc/details/compacting/forwarding.hpp>

#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace allocators {

class mpool_allocator : private utils::noncopyable, private utils::nonmovable
{
    typedef managed_pool_chunk descriptor_t;

    typedef allocators::intrusive_list_allocator<
            allocators::freelist_pool_chunk
            , allocators::default_allocator
            , allocators::single_chunk_with_search_cache
            , utils::dummy_mutex
    > chunk_pool_t;

    typedef std::list<descriptor_t, stl_adapter<descriptor_t, chunk_pool_t>> descriptor_list_t;
    typedef typename descriptor_list_t::iterator iterator_t;
public:
    typedef gc_alloc_response pointer_type;
    typedef stateful_alloc_tag alloc_tag;

    mpool_allocator();

    gc_alloc_response allocate(const gc_alloc_request& rqst);

    gc_heap_stat collect();
    void fix(const compacting::forwarding& frwd);
private:
    struct object_meta
    {
        const gc_type_meta* m_tmeta;
        size_t m_obj_cnt;
    };

    static gc_alloc_response make_obj(byte* ptr, memory_descriptor* descr, const gc_alloc_request& rqst);

    gc_alloc_response try_expand_and_allocate(size_t size, const gc_alloc_request& rqst, bool call_gc);
    gc_alloc_response stack_allocation(size_t size, const gc_alloc_request& rqst);
    gc_alloc_response freelist_allocation(size_t size, const gc_alloc_request& rqst);

    mpool_allocator::iterator_t create_descriptor(byte* blk, size_t blk_size, size_t cell_size);
    iterator_t destroy_descriptor(iterator_t it);

    std::pair<byte*, size_t> allocate_block(size_t cell_size);
    void deallocate_block(byte* ptr, size_t size);

    bool contains(byte* ptr) const;

    // remove unused chunks and calculate some statistic
    void shrink(gc_heap_stat& stat);

    descriptor_list_t m_descrs;
    byte** m_freelist;
    byte*  m_top;
    byte*  m_end;
    size_t m_cell_idx;
};

}}}

#endif //DIPLOMA_MPOOL_ALLOCATOR_HPP
