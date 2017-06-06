#ifndef ALLOCGC_HEAP_H
#define ALLOCGC_HEAP_H

#include <utility>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <memory>
#include <unordered_map>

#include <liballocgc/gc_alloc.hpp>
#include <liballocgc/details/gc_interface.hpp>
#include <liballocgc/details/constants.hpp>

#include <liballocgc/details/allocators/gc_core_allocator.hpp>
#include <liballocgc/details/allocators/gc_lo_allocator.hpp>
#include <liballocgc/details/allocators/gc_so_allocator.hpp>

#include <liballocgc/details/collectors/static_root_set.hpp>

#include <liballocgc/details/compacting/forwarding.hpp>

//#include <liballocgc/details/threads/world_snapshot.hpp>
#include <liballocgc/details/utils/safepoint_lock.hpp>
#include <liballocgc/details/utils/dummy_mutex.hpp>

#include <liballocgc/details/utils/utility.hpp>

namespace allocgc { namespace details {

namespace threads {
class world_snapshot;
}

class gc_heap : public utils::noncopyable, public utils::nonmovable
{
    typedef allocators::gc_core_allocator   core_alloc_t;
    typedef allocators::gc_so_allocator     so_alloc_t;
    typedef allocators::gc_lo_allocator     lo_alloc_t;
public:
    typedef so_alloc_t tlab;

    explicit gc_heap(gc_launcher* launcher);

    gc_alloc::response allocate(const gc_alloc::request& rqst);

    tlab* allocate_tlab(std::thread::id thrd_id);

    gc_collect_stat collect(
            const threads::world_snapshot& snapshot,
            size_t threads_available,
            collectors::static_root_set* static_roots
    );

    gc_memstat stats();

    void shrink();

    void set_limit(size_t limit);
private:
    typedef std::unordered_map<std::thread::id, so_alloc_t> tlab_map_t;

    core_alloc_t    m_core_alloc;
    lo_alloc_t      m_loa;
    tlab_map_t      m_tlab_map;
    std::mutex      m_mutex;
};

}}

#endif //ALLOCGC_HEAP_H
