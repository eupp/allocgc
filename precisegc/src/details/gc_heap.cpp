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

const size_t gc_heap::TLAB_SIZE = 2 * 1024 * 1024;

gc_heap::gc_heap()
{}

allocators::gc_alloc_response gc_heap::allocate(const allocators::gc_alloc_request& rqst)
{
    if (rqst.alloc_size() <= LARGE_CELL_SIZE) {
        return allocate_on_tlab(rqst);
    } else {
        return m_loa.allocate(rqst);
    }
}

gc_heap_stat gc_heap::collect(const threads::world_snapshot& snapshot, gc_gen gen, size_t threads_available)
{
    forwarding frwd;
    utils::static_thread_pool thread_pool(threads_available);

    gc_heap_stat stat;
    if (gen == GC_OLD_GEN) {
        stat += m_old_soa.collect(frwd, nullptr, thread_pool);
        stat += m_loa.collect(frwd);
    }
    for (auto& kv: m_tlab_map) {
        stat += kv.second.alloc.collect(frwd, &m_old_soa, thread_pool);
        kv.second.size = 0;
    }

    if (stat.mem_copied > 0) {
//        for (auto& kv: m_tlab_map) {
//            kv.second.fix(frwd, thread_pool);
//        }
        m_old_soa.fix(frwd, thread_pool);
        if (gen == GC_OLD_GEN) {
            m_loa.fix(frwd);
        }
        snapshot.fix_roots(frwd);
    }

//    for (auto& kv: m_tlab_map) {
//        kv.second.finalize();
//    }
    m_old_soa.finalize();
    if (gen == GC_OLD_GEN) {
        m_loa.finalize();
    }

    return stat;
}

allocators::gc_alloc_response gc_heap::allocate_on_tlab(const allocators::gc_alloc_request& rqst)
{
    static thread_local gc_heap::tlab_t& tlab = get_tlab();
    tlab.size += rqst.alloc_size();
    if (tlab.size > TLAB_SIZE) {
        gc_options options;
        options.kind = gc_kind::COLLECT;
        options.gen = GC_YOUNG_GEN;
        gc_initiation_point(initiation_point_type::TLAB_LIMIT_EXCEEDED, options);
    }
    return tlab.alloc.allocate(rqst);
}

gc_heap::tlab_t& gc_heap::get_tlab()
{
    std::lock_guard<std::mutex> lock(m_tlab_map_mutex);
    tlab_t& tlab = m_tlab_map[std::this_thread::get_id()];
    tlab.size = 0;
    return tlab;
}

}}