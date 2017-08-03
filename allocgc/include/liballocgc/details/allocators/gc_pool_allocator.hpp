#ifndef ALLOCGC_GC_POOL_ALLOCATOR_HPP
#define ALLOCGC_GC_POOL_ALLOCATOR_HPP

#include <list>
#include <cstring>
#include <utility>

#include <boost/intrusive/slist.hpp>

#include <liballocgc/details/allocators/pool_allocator.hpp>
#include <liballocgc/details/allocators/gc_bucket_policy.hpp>
#include <liballocgc/details/allocators/gc_pool_descriptor.hpp>
#include <liballocgc/details/allocators/default_allocator.hpp>
#include <liballocgc/details/allocators/gc_core_allocator.hpp>
#include <liballocgc/details/allocators/allocator_tag.hpp>
#include <liballocgc/details/allocators/stl_adapter.hpp>

#include <liballocgc/details/utils/flatten_range.hpp>
#include <liballocgc/details/utils/utility.hpp>

#include <liballocgc/details/compacting/forwarding.hpp>

#include <liballocgc/gc_alloc.hpp>
#include <liballocgc/details/constants.hpp>
#include <liballocgc/details/logging.hpp>

namespace allocgc { namespace details { namespace allocators {

class gc_pool_allocator : private utils::noncopyable, private utils::nonmovable
{
    typedef gc_pool_descriptor_base descriptor_t;

    typedef boost::intrusive::slist<descriptor_t> descriptor_list_t;
    typedef typename descriptor_list_t::iterator iterator_t;
public:
    typedef stateful_alloc_tag alloc_tag;

    gc_pool_allocator();
    ~gc_pool_allocator();

    void init(const gc_bucket_policy* bucket_policy, gc_core_allocator* core_alloc);

    gc_alloc::response allocate(const gc_alloc::request& rqst, size_t aligned_size);

    gc_collect_stat collect(compacting::forwarding& frwd);
    void fix(const compacting::forwarding& frwd);
    void finalize();

    gc_memstat stats();

    bool empty() const;
private:
    static constexpr double RESIDENCY_COMPACTING_THRESHOLD = 0.5;
    static constexpr double RESIDENCY_NON_COMPACTING_THRESHOLD = 0.9;
    static constexpr double RESIDENCY_EPS = 0.1;

    gc_alloc::response try_expand_and_allocate(size_t size, const gc_alloc::request& rqst, size_t attempt_num);
    gc_alloc::response stack_allocation(size_t size, const gc_alloc::request& rqst);
    gc_alloc::response freelist_allocation(size_t size, const gc_alloc::request& rqst);
    gc_alloc::response init_cell(byte* box_addr, const gc_alloc::request& rqst, descriptor_t* descr);

    void create_descriptor(byte* blk, size_t blk_size, size_t cell_size);

    void destroy_descriptor(descriptor_t& descr);
    gc_pool_allocator::iterator_t destroy_descriptor(iterator_t it, iterator_t prev);

    std::pair<byte*, size_t> allocate_block(size_t cell_size);
    void deallocate_block(byte* ptr, size_t size);

    bool contains(byte* ptr) const;

    bool is_compaction_required(double residency) const;

    // remove unused chunks and calculate some statistic
    double shrink(gc_collect_stat& stat);
    void sweep(gc_collect_stat& stat);
    void compact(compacting::forwarding& frwd, gc_collect_stat& stat);

    size_t sweep(descriptor_t& descr, bool add_to_freelist);

    void insert_into_freelist(byte* ptr);

    const gc_bucket_policy* m_bucket_policy;
    gc_core_allocator* m_core_alloc;
    descriptor_list_t m_descrs;
    byte** m_freelist;
    byte*  m_top;
    byte*  m_end;
    double m_prev_residency;
};

}}}

#endif //ALLOCGC_GC_POOL_ALLOCATOR_HPP
