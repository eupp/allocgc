#include "gc_heap.h"

#include <cassert>
#include <array>
#include <vector>
#include <functional>

#include <libprecisegc/details/threads/this_managed_thread.hpp>
#include <libprecisegc/details/utils/static_thread_pool.hpp>
#include <libprecisegc/details/utils/math.hpp>
#include <libprecisegc/details/gc_compact.h>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details {

gc_heap::gc_heap(gc_compacting compacting)
    : m_compacting(compacting)
{}

managed_ptr gc_heap::allocate(size_t size)
{
    if (size <= LARGE_CELL_SIZE) {
        return threads::this_managed_thread::allocate_on_tlab(size);
    }
    throw gc_bad_alloc();
}

gc_sweep_stat gc_heap::sweep(const threads::world_snapshot& snapshot, size_t threads_available)
{
    gc_sweep_stat stat;

    forwarding frwd;
    size_t freed_in_tlabs = snapshot.sweep_tlabs(m_compacting, frwd);

    if (m_compacting == gc_compacting::ENABLED) {
        snapshot.fix_pointers_in_tlabs(frwd);
        snapshot.fix_roots(frwd);
    }

    snapshot.unmark_tlabs();

    stat.shrunk = freed_in_tlabs;
    stat.swept  = 0;

    return stat;
}

//gc_heap::forwarding gc_heap::compact()
//{
//    logging::info() << "Compacting memory...";
//
//    forwarding frwd;
//    auto& bp = m_alloc.get_bucket_policy();
//    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
//        auto rng = m_alloc.memory_range(i);
//        two_finger_compact(rng, bp.bucket_size(i), frwd);
//    }
//    return frwd;
//}
//
//gc_heap::forwarding gc_heap::parallel_compact(size_t threads_num)
//{
//    logging::info() << "Compacting memory (in parallel with " << threads_num << " threads)...";
//
//    utils::static_thread_pool thread_pool(threads_num);
//    std::array<std::function<void()>, SEGREGATED_STORAGE_SIZE> tasks;
//    std::array<forwarding, SEGREGATED_STORAGE_SIZE> forwardings;
//
//    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
//        tasks[i] = [this, i, &forwardings] {
//            auto& bp = m_alloc.get_bucket_policy();
//            auto rng = m_alloc.memory_range(i);
//            two_finger_compact(rng, bp.bucket_size(i), forwardings[i]);
//        };
//    }
//    thread_pool.run(tasks.begin(), tasks.end());
//
//    forwarding frwd = forwardings[0];
//    for (size_t i = 1; i < SEGREGATED_STORAGE_SIZE; ++i) {
//        frwd.join(forwardings[i]);
//    }
//    return frwd;
//}
//
//void gc_heap::fix_pointers(const gc_heap::forwarding& frwd)
//{
//    logging::info() << "Fixing pointers...";
//
//    auto& bp = m_alloc.get_bucket_policy();
//    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
//        auto rng = m_alloc.memory_range(i);
//        ::precisegc::details::fix_pointers(rng.begin(), rng.end(), bp.bucket_size(i), frwd);
//    }
//}
//
//void gc_heap::parallel_fix_pointers(const forwarding& frwd, size_t threads_num)
//{
//    logging::info() << "Fixing pointers (in parallel with " << threads_num << " threads)...";
//
//    utils::static_thread_pool thread_pool(threads_num);
//    std::array<std::function<void()>, SEGREGATED_STORAGE_SIZE> tasks;
//
//    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
//        tasks[i] = [this, i, &frwd] {
//            auto& bp = m_alloc.get_bucket_policy();
//            auto rng = m_alloc.memory_range(i);
//            ::precisegc::details::fix_pointers(rng.begin(), rng.end(), bp.bucket_size(i), frwd);
//        };
//    }
//    thread_pool.run(tasks.begin(), tasks.end());
//}

}}