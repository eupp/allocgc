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

gc_alloc_descriptor gc_heap::allocate(size_t size)
{
    if (size <= LARGE_CELL_SIZE) {
        return allocate_on_tlab(size);
    } else {
        return m_loa.allocate(size);
    }
}

gc_heap::collect_stats gc_heap::collect(const threads::world_snapshot& snapshot, size_t threads_available)
{
    if (threads_available > 1) {
        return parallel_collect(snapshot, threads_available);
    } else {
        return serial_collect(snapshot);
    }
}

gc_heap::collect_stats gc_heap::serial_collect(const threads::world_snapshot& snapshot)
{
    size_t freed  = 0;
    size_t copied = 0;

    forwarding frwd;

    freed += m_loa.shrink();

    std::lock_guard<std::mutex> lock(m_tlab_map_mutex);
    for (auto it = m_tlab_map.begin(); it != m_tlab_map.end(); ) {
        auto& tlab = it->second;
        for (size_t i = 0; i < tlab_bucket_policy::BUCKET_COUNT; ++i) {
            auto res = compact_heap_part(i, tlab, frwd);
            freed  += res.first;
            copied += res.second;
        }

        if (tlab.empty() && snapshot.lookup_thread(it->first) == nullptr) {
            it = m_tlab_map.erase(it);
        } else {
            ++it;
        }
    }

    if (copied > 0) {
        fix_pointers(frwd);
        snapshot.fix_roots(frwd);
    }

    collect_stats stats;
    stats.mem_swept  = freed;
    stats.mem_copied = copied;
    return stats;
}

gc_heap::collect_stats gc_heap::parallel_collect(const threads::world_snapshot& snapshot, size_t threads_available)
{
    std::atomic<size_t> freed{0};
    std::atomic<size_t> copied{0};

    forwarding frwd;

    freed += m_loa.shrink();

    utils::static_thread_pool thread_pool(threads_available);
    std::vector<std::function<void()>> tasks;

    std::lock_guard<std::mutex> lock(m_tlab_map_mutex);
    for (auto it = m_tlab_map.begin(); it != m_tlab_map.end(); ++it) {
        auto& tlab = it->second;
        for (size_t i = 0; i < tlab_bucket_policy::BUCKET_COUNT; ++i) {
            tasks.push_back([this, i, &tlab, &frwd, &freed, &copied] {
                auto res = compact_heap_part(i, tlab, frwd);
                freed  += res.first;
                copied += res.second;
            });
        }
    }
    thread_pool.run(tasks.begin(), tasks.end());

    for (auto it = m_tlab_map.begin(); it != m_tlab_map.end(); ) {
        auto& tlab = it->second;
        if (tlab.empty() && snapshot.lookup_thread(it->first) == nullptr) {
            it = m_tlab_map.erase(it);
        } else {
            ++it;
        }
    }

    if (copied > 0) {
        parallel_fix_pointers(frwd, threads_available);
        snapshot.fix_roots(frwd);
    }

    collect_stats stats;
    stats.mem_swept  = freed;
    stats.mem_copied = copied;
    return stats;
}

gc_alloc_descriptor gc_heap::allocate_on_tlab(size_t size)
{
    assert(size <= LARGE_CELL_SIZE);
    static thread_local tlab_t& tlab = get_tlab();
    return tlab.allocate(std::max(size, MIN_CELL_SIZE));
}

gc_heap::tlab_t& gc_heap::get_tlab()
{
    std::lock_guard<std::mutex> lock(m_tlab_map_mutex);
    return m_tlab_map[std::this_thread::get_id()];
}

void gc_heap::update_heap_part_stat(size_t bucket_ind, tlab_t& tlab, const heap_part_stat& stats)
{
    m_heap_stat_map[&tlab.get_bucket_alloc(bucket_ind)] = stats;
}

std::pair<size_t, size_t> gc_heap::compact_heap_part(size_t bucket_ind, tlab_t& tlab, forwarding& frwd)
{
    compacting::two_finger_compactor compactor;
    fixsize_alloc_t& bucket_alloc = tlab.get_bucket_alloc(bucket_ind);

    heap_part_stat curr_stats = bucket_alloc.collect();
    heap_part_stat prev_stats = m_heap_stat_map[&bucket_alloc];

    size_t freed  = curr_stats.mem_shrunk;
    size_t copied = 0;

    if (is_compacting_required(curr_stats, prev_stats)) {
        auto rng = bucket_alloc.memory_range();
        copied += compactor(rng, frwd);
        curr_stats.residency = bucket_alloc.residency();
    }
    bucket_alloc.sweep();
    update_heap_part_stat(bucket_ind, tlab, curr_stats);

    return std::make_pair(freed, copied);
}

void gc_heap::fix_pointers(const gc_heap::forwarding& frwd)
{
    logging::info() << "Fixing pointers...";

    for (auto& kv: m_tlab_map) {
        auto& tlab = kv.second;
        for (size_t i = 0; i < tlab_bucket_policy::BUCKET_COUNT; ++i) {
            auto rng = tlab.memory_range(i);
            compacting::fix_ptrs(rng.begin(), rng.end(), frwd, tlab_bucket_policy::bucket_size(i));
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
                    compacting::fix_ptrs(rng.begin(), rng.end(), frwd, tlab_bucket_policy::bucket_size(i));
                });
            }
        }
    }
    thread_pool.run(tasks.begin(), tasks.end());
}

bool gc_heap::is_compacting_required(const heap_part_stat& curr_stats, const heap_part_stat& prev_stats)
{
    return curr_stats.residency < RESIDENCY_COMPACTING_THRESHOLD
           || (curr_stats.residency < RESIDENCY_NON_COMPACTING_THRESHOLD
               && std::abs(curr_stats.residency - prev_stats.residency) < RESIDENCY_EPS);
}

}}