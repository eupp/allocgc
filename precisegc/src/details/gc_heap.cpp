#include "gc_heap.hpp"

#include <cassert>
#include <vector>
#include <functional>

#include <libprecisegc/details/compacting/fix_ptrs.hpp>
#include <libprecisegc/details/compacting/two_finger_compactor.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/this_managed_thread.hpp>
#include <libprecisegc/details/utils/static_thread_pool.hpp>
#include <libprecisegc/details/utils/math.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details {

gc_heap::gc_heap(gc_compacting compacting)
    : m_compacting(compacting)
{}

gc_alloc_response gc_heap::allocate(const gc_alloc_request& rqst)
{
    if (rqst.alloc_size() <= LARGE_CELL_SIZE) {
        return allocate_on_tlab(rqst);
    } else {
        return m_loa.allocate(rqst);
    }
}

gc_heap_stat gc_heap::collect(const threads::world_snapshot& snapshot, size_t threads_available)
{
    forwarding frwd;
    utils::static_thread_pool thread_pool(threads_available);

    gc_heap_stat stat;
    for (auto& kv: m_tlab_map) {
        stat += kv.second.collect(frwd, thread_pool);
    }
    stat += m_loa.collect(frwd);

    if (stat.mem_copied > 0) {
        for (auto& kv: m_tlab_map) {
            kv.second.fix(frwd, thread_pool);
        }
        m_loa.fix(frwd);
        snapshot.fix_roots(frwd);
    }

    return stat;
}

gc_alloc_response gc_heap::allocate_on_tlab(const gc_alloc_request& rqst)
{
    static thread_local mso_alloc_t& tlab = get_tlab();
    return tlab.allocate(rqst);
}

gc_heap::mso_alloc_t& gc_heap::get_tlab()
{
    std::lock_guard<std::mutex> lock(m_tlab_map_mutex);
    return m_tlab_map[std::this_thread::get_id()];
}

}}