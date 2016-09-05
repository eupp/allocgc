#ifndef DIPLOMA_HEAP_H
#define DIPLOMA_HEAP_H

#include <utility>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <memory>
#include <unordered_map>

#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/page_allocator.hpp>
#include <libprecisegc/details/allocators/bucket_allocator.hpp>
#include <libprecisegc/details/allocators/list_allocator.hpp>
#include <libprecisegc/details/allocators/intrusive_list_allocator.hpp>
#include <libprecisegc/details/allocators/intrusive_list_pool_allocator.hpp>
#include <libprecisegc/details/allocators/managed_large_object_descriptor.hpp>
#include <libprecisegc/details/allocators/managed_pool_chunk.hpp>
#include <libprecisegc/details/allocators/cache_policies.hpp>
#include <libprecisegc/details/allocators/pow2_bucket_policy.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/utils/safepoint_lock.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/forwarding.h>
#include <libprecisegc/details/object_meta.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/constants.hpp>

namespace precisegc { namespace details {

class gc_heap : public utils::noncopyable, public utils::nonmovable
{
    typedef allocators::pow2_bucket_policy<MIN_CELL_SIZE_BITS_CNT, LARGE_CELL_SIZE_BITS_CNT> tlab_bucket_policy;

    typedef allocators::intrusive_list_pool_allocator<
            allocators::freelist_pool_chunk, allocators::default_allocator
        > chunk_pool_t;

    typedef allocators::list_allocator<
            allocators::managed_pool_chunk,
            allocators::page_allocator,
            chunk_pool_t,
            allocators::single_chunk_cache,
            utils::dummy_mutex
        > fixsize_alloc_t;

    typedef allocators::bucket_allocator<
            fixsize_alloc_t,
            tlab_bucket_policy
        > tlab_t;

    typedef allocators::intrusive_list_allocator<
            allocators::managed_large_object_descriptor,
            allocators::page_allocator,
            allocators::always_expand,
            utils::safepoint_lock<std::recursive_mutex>
        > loa_t;

    static_assert(std::is_same<typename tlab_t::pointer_type, typename loa_t::pointer_type>::value,
                  "Large and small object allocators should have same pointer_type");

    typedef intrusive_forwarding forwarding;
public:
    struct collect_stats
    {
        size_t      mem_swept;
        size_t      mem_copied;
    };

    gc_heap(gc_compacting compacting);

    gc_pointer_type allocate(size_t size);

    collect_stats collect(const threads::world_snapshot& snapshot, size_t threads_available);
private:
    typedef std::unordered_map<std::thread::id, tlab_t> tlab_map_t;

    gc_pointer_type allocate_on_tlab(size_t size);
    tlab_t& get_tlab();

    size_t shrink(const threads::world_snapshot& snapshot);

    std::pair<forwarding, size_t> compact();
    std::pair<forwarding, size_t> parallel_compact(size_t threads_num);

    void fix_pointers(const forwarding& frwd);
    void parallel_fix_pointers(const forwarding& frwd, size_t threads_num);

    void unmark();

    loa_t m_loa;
    tlab_map_t m_tlab_map;
    std::mutex m_tlab_map_mutex;
    gc_compacting m_compacting;
};

}}

#endif //DIPLOMA_HEAP_H
