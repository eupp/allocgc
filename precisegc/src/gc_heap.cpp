#include "gc_heap.h"

#include <cassert>
#include <array>
#include <vector>
#include <functional>

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
    return m_alloc.allocate(size);
}

gc_sweep_stat gc_heap::sweep(const threads::world_snapshot& snapshot, size_t threads_available)
{
    gc_sweep_stat stat;

    logging::info() << "Shrinking memory...";
    stat.shrunk = m_alloc.shrink();
    stat.swept  = 0;

    if (m_compacting == gc_compacting::ENABLED) {
        if (threads_available > 1) {
            forwarding frwd = parallel_compact(threads_available);
            parallel_fix_pointers(frwd, threads_available);
            fix_roots(snapshot, frwd);
        } else {
            forwarding frwd = compact();
            fix_pointers(frwd);
            fix_roots(snapshot, frwd);
        }
    }

    m_alloc.apply_to_chunks([] (allocators::managed_pool_chunk& chunk) {
        chunk.unmark();
    });

    return stat;
}

gc_heap::forwarding gc_heap::compact()
{
    logging::info() << "Compacting memory...";

    forwarding frwd;
    auto& bp = m_alloc.get_bucket_policy();
    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
        auto rng = m_alloc.memory_range(i);
        two_finger_compact(rng, bp.bucket_size(i), frwd);
    }
    return frwd;
}

gc_heap::forwarding gc_heap::parallel_compact(size_t threads_num)
{
    logging::info() << "Compacting memory (in parallel with " << threads_num << " threads)...";

    utils::static_thread_pool thread_pool(threads_num);
    std::array<std::function<void()>, SEGREGATED_STORAGE_SIZE> tasks;
    std::array<forwarding, SEGREGATED_STORAGE_SIZE> forwardings;

    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
        tasks[i] = [this, i, &forwardings] {
            auto& bp = m_alloc.get_bucket_policy();
            auto rng = m_alloc.memory_range(i);
            two_finger_compact(rng, bp.bucket_size(i), forwardings[i]);
        };
    }
    thread_pool.run(tasks.begin(), tasks.end());

    forwarding frwd = forwardings[0];
    for (size_t i = 1; i < SEGREGATED_STORAGE_SIZE; ++i) {
        frwd.join(forwardings[i]);
    }
    return frwd;
}

void gc_heap::fix_pointers(const gc_heap::forwarding& frwd)
{
    logging::info() << "Fixing pointers...";

    auto& bp = m_alloc.get_bucket_policy();
    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
        auto rng = m_alloc.memory_range(i);
        ::precisegc::details::fix_pointers(rng.begin(), rng.end(), bp.bucket_size(i), frwd);
    }
}

void gc_heap::parallel_fix_pointers(const forwarding& frwd, size_t threads_num)
{
    logging::info() << "Fixing pointers (in parallel with " << threads_num << " threads)...";

    utils::static_thread_pool thread_pool(threads_num);
    std::array<std::function<void()>, SEGREGATED_STORAGE_SIZE> tasks;

    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
        tasks[i] = [this, i, &frwd] {
            auto& bp = m_alloc.get_bucket_policy();
            auto rng = m_alloc.memory_range(i);
            ::precisegc::details::fix_pointers(rng.begin(), rng.end(), bp.bucket_size(i), frwd);
        };
    }
    thread_pool.run(tasks.begin(), tasks.end());
}

void gc_heap::fix_roots(const threads::world_snapshot& snapshot, const forwarding& frwd)
{
    logging::info() << "Fixing roots...";

    snapshot.fix_roots(frwd);
}

}}