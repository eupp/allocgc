#ifndef DIPLOMA_HEAP_H
#define DIPLOMA_HEAP_H

#include <utility>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <memory>
#include <unordered_map>

#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/core_allocator.hpp>
#include <libprecisegc/details/allocators/bucket_allocator.hpp>
#include <libprecisegc/details/allocators/mso_allocator.hpp>
#include <libprecisegc/details/allocators/intrusive_list_allocator.hpp>
#include <libprecisegc/details/allocators/managed_object_descriptor.hpp>
#include <libprecisegc/details/allocators/cache_policies.hpp>
#include <libprecisegc/details/allocators/pow2_bucket_policy.hpp>
#include <libprecisegc/details/compacting/forwarding.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/utils/safepoint_lock.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/collectors/traceable_object_meta.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/compacting/forwarding.hpp>

namespace precisegc { namespace details {

class gc_heap : public utils::noncopyable, public utils::nonmovable
{
    typedef allocators::pow2_bucket_policy<MIN_CELL_SIZE_BITS_CNT, LARGE_CELL_SIZE_BITS_CNT> tlab_bucket_policy;

    typedef allocators::intrusive_list_allocator<
              allocators::freelist_pool_chunk
            , allocators::default_allocator
            , allocators::single_chunk_with_search_cache
            , utils::dummy_mutex
        > chunk_pool_t;

    typedef allocators::mso_allocator fixsize_alloc_t;

    typedef allocators::bucket_allocator<
            fixsize_alloc_t,
            tlab_bucket_policy
        > tlab_t;

    typedef allocators::intrusive_list_allocator<
            allocators::managed_object_descriptor,
            allocators::core_allocator,
            allocators::always_expand,
            utils::safepoint_lock<std::recursive_mutex>
        > loa_t;

    static_assert(std::is_same<typename tlab_t::pointer_type, typename loa_t::pointer_type>::value,
                  "Large and small object allocators should have same pointer_type");

    typedef compacting::forwarding forwarding;
public:
    struct collect_stats
    {
        size_t      mem_swept;
        size_t      mem_copied;
    };

    gc_heap(gc_compacting compacting);

    gc_alloc_descriptor allocate(size_t size);

    collect_stats collect(const threads::world_snapshot& snapshot, size_t threads_available);
private:
    static constexpr double RESIDENCY_COMPACTING_THRESHOLD = 0.5;
    static constexpr double RESIDENCY_NON_COMPACTING_THRESHOLD = 0.9;
    static constexpr double RESIDENCY_EPS = 0.1;

    typedef std::unordered_map<std::thread::id, tlab_t> tlab_map_t;
    typedef std::unordered_map<fixsize_alloc_t*, heap_part_stat> heap_stat_map_t;

    static bool is_compacting_required(const heap_part_stat& curr_stats, const heap_part_stat& prev_stats);

    gc_alloc_descriptor allocate_on_tlab(size_t size);
    tlab_t& get_tlab();

    collect_stats serial_collect(const threads::world_snapshot& snapshot);
    collect_stats parallel_collect(const threads::world_snapshot& snapshot, size_t threads_available);

    std::pair<size_t, size_t> compact_heap_part(size_t bucket_ind, tlab_t& tlab, forwarding& frwd);
    void update_heap_part_stat(size_t bucket_ind, tlab_t& tlab, const heap_part_stat& stats);

    void fix_pointers(const forwarding& frwd);
    void parallel_fix_pointers(const forwarding& frwd, size_t threads_num);

    loa_t m_loa;
    tlab_map_t m_tlab_map;
    heap_stat_map_t m_heap_stat_map;
    std::mutex m_tlab_map_mutex;
    gc_compacting m_compacting;
};

}}

#endif //DIPLOMA_HEAP_H
