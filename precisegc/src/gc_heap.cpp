#include "gc_heap.h"

#include <cassert>
#include <vector>
#include <functional>

#include <libprecisegc/details/threads/thread_manager.hpp>
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
        return allocate_on_tlab(size);
    }
    throw gc_bad_alloc();
}

gc_sweep_stat gc_heap::sweep(const threads::world_snapshot& snapshot, size_t threads_available)
{
    gc_sweep_stat stat;

    size_t freed_in_tlabs = shrink(snapshot);

    if (m_compacting == gc_compacting::ENABLED) {
        if (threads_available > 1) {
            forwarding frwd = parallel_compact(threads_available);
            parallel_fix_pointers(frwd, threads_available);
            snapshot.fix_roots(frwd);
        } else {
            forwarding frwd = compact();
            fix_pointers(frwd);
            snapshot.fix_roots(frwd);
        }
    }

    unmark();

    stat.shrunk = freed_in_tlabs;
    stat.swept  = 0;

    return stat;
}

managed_ptr gc_heap::allocate_on_tlab(size_t size)
{
    assert(size <= LARGE_CELL_SIZE);
    static thread_local tlab_t& tlab = get_tlab();
    return tlab.allocate(size);
}

gc_heap::tlab_t& gc_heap::get_tlab()
{
    std::lock_guard<std::mutex> lock(m_tlab_map_mutex);
    return m_tlab_map[std::this_thread::get_id()];
}

size_t gc_heap::shrink(const threads::world_snapshot& snapshot)
{
    logging::info() << "Clearing empty pages...";

    size_t freed = 0;
    for (auto it = m_tlab_map.begin(); it != m_tlab_map.end(); ) {
        freed += it->second.shrink();
        if (it->second.empty() && snapshot.lookup_thread(it->first) == nullptr) {
            it = m_tlab_map.erase(it);
        } else {
            ++it;
        }
    }
    return freed;
}

gc_heap::forwarding gc_heap::compact()
{
    logging::info() << "Compacting memory...";

    forwarding frwd;
    for (auto& kv: m_tlab_map) {
        auto& tlab = kv.second;
        for (size_t i = 0; i < tlab_bucket_policy::BUCKET_COUNT; ++i) {
            auto rng = tlab.memory_range(i);
            two_finger_compact(rng, tlab_bucket_policy::bucket_size(i), frwd);
        }
    }
    return frwd;
}

gc_heap::forwarding gc_heap::parallel_compact(size_t threads_num)
{
    logging::info() << "Compacting memory (in parallel with " << threads_num << " threads)...";

    utils::static_thread_pool thread_pool(threads_num);
    std::vector<std::function<void()>> tasks;

    forwarding frwd;
    for (auto& kv: m_tlab_map) {
        auto& tlab = kv.second;
        for (size_t i = 0; i < tlab_bucket_policy::BUCKET_COUNT; ++i) {
            auto rng = tlab.memory_range(i);
            if (!rng.empty()) {
                tasks.emplace_back([this, rng, i, &frwd] {
                    two_finger_compact(rng, tlab_bucket_policy::bucket_size(i), frwd);
                });
            }
        }
    }
    thread_pool.run(tasks.begin(), tasks.end());
    return frwd;
}

void gc_heap::fix_pointers(const gc_heap::forwarding& frwd)
{
    logging::info() << "Fixing pointers...";

    for (auto& kv: m_tlab_map) {
        auto& tlab = kv.second;
        for (size_t i = 0; i < tlab_bucket_policy::BUCKET_COUNT; ++i) {
            auto rng = tlab.memory_range(i);
            ::precisegc::details::fix_pointers(rng.begin(), rng.end(), tlab_bucket_policy::bucket_size(i), frwd);
        }
    }
}

void gc_heap::parallel_fix_pointers(const forwarding& frwd, size_t threads_num)
{
    logging::info() << "Fixing pointers (in parallel with " << threads_num << " threads)...";

    utils::static_thread_pool thread_pool(threads_num);
    std::vector<std::function<void()>> tasks;

    for (auto& kv: m_tlab_map) {
        auto& tlab = kv.second;
        for (size_t i = 0; i < tlab_bucket_policy::BUCKET_COUNT; ++i) {
            auto rng = tlab.memory_range(i);
            if (!rng.empty()) {
                tasks.emplace_back([this, rng, i, &frwd] {
                    ::precisegc::details::fix_pointers(rng.begin(), rng.end(), tlab_bucket_policy::bucket_size(i), frwd);
                });
            }
        }
    }
    thread_pool.run(tasks.begin(), tasks.end());
}

void gc_heap::unmark()
{
    for (auto& kv: m_tlab_map) {
        kv.second.apply_to_chunks([] (allocators::managed_pool_chunk& chunk) {
            chunk.unmark();
        });
    }
}

}}