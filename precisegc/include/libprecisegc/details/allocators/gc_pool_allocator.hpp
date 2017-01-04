#ifndef DIPLOMA_GC_POOL_ALLOCATOR_HPP
#define DIPLOMA_GC_POOL_ALLOCATOR_HPP

#include <list>
#include <cstring>
#include <utility>

#include <libprecisegc/details/allocators/pool_allocator.hpp>
#include <libprecisegc/details/allocators/gc_pool_descriptor.hpp>
#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/gc_core_allocator.hpp>
#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/allocators/stl_adapter.hpp>

#include <libprecisegc/details/utils/flatten_range.hpp>
#include <libprecisegc/details/utils/utility.hpp>

#include <libprecisegc/details/compacting/forwarding.hpp>

#include <libprecisegc/details/allocators/gc_alloc_messaging.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace allocators {

class gc_pool_allocator : private utils::noncopyable, private utils::nonmovable
{
    typedef gc_pool_descriptor descriptor_t;

    typedef allocators::pool_allocator<
              allocators::default_allocator
            , utils::dummy_mutex
        > chunk_pool_t;

    typedef std::list<descriptor_t, stl_adapter<descriptor_t, chunk_pool_t>> descriptor_list_t;
    typedef typename descriptor_list_t::iterator iterator_t;
public:
    typedef utils::flattened_range<iterator_t> memory_range_type;
    typedef gc_alloc_response pointer_type;
    typedef stateful_alloc_tag alloc_tag;

    gc_pool_allocator();
    ~gc_pool_allocator();

    gc_alloc_response allocate(const gc_alloc_request& rqst, size_t aligned_size);

    gc_heap_stat collect(compacting::forwarding& frwd);
    void fix(const compacting::forwarding& frwd);
    void finalize();

    bool empty() const;

    memory_range_type memory_range();
private:
    static constexpr double RESIDENCY_COMPACTING_THRESHOLD = 0.5;
    static constexpr double RESIDENCY_NON_COMPACTING_THRESHOLD = 0.9;
    static constexpr double RESIDENCY_EPS = 0.1;

    gc_alloc_response try_expand_and_allocate(size_t size, const gc_alloc_request& rqst, size_t attempt_num);
    gc_alloc_response stack_allocation(size_t size, const gc_alloc_request& rqst);
    gc_alloc_response freelist_allocation(size_t size, const gc_alloc_request& rqst);
    gc_alloc_response init_cell(byte* cell_start, const gc_alloc_request& rqst, descriptor_t* descr);

    gc_pool_allocator::iterator_t create_descriptor(byte* blk, size_t blk_size, size_t cell_size);
    iterator_t destroy_descriptor(iterator_t it);

    std::pair<byte*, size_t> allocate_block(size_t cell_size);
    void deallocate_block(byte* ptr, size_t size);

    bool contains(byte* ptr) const;

    bool is_compaction_required(const gc_heap_stat& stat) const;

    // remove unused chunks and calculate some statistic
    void shrink(gc_heap_stat& stat);
    void sweep(gc_heap_stat& stat);
    void compact(compacting::forwarding& frwd, gc_heap_stat& stat);

    size_t sweep(descriptor_t& descr, bool add_to_freelist);

    void insert_into_freelist(byte* ptr);

    descriptor_list_t m_descrs;
    byte** m_freelist;
    byte*  m_top;
    byte*  m_end;
    double m_prev_residency;
};

}}}

#endif //DIPLOMA_GC_POOL_ALLOCATOR_HPP
