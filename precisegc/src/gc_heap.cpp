#include "gc_heap.hpp"

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

gc_pointer_type gc_heap::allocate(size_t size)
{
    if (size <= LARGE_CELL_SIZE) {
        return allocate_on_tlab(size);
    } else {
        return utils::make_block_ptr(m_loa.allocate(size), size);
    }
}

gc_heap::collect_stats gc_heap::collect(const threads::world_snapshot& snapshot, size_t threads_available)
{
    collect_stats stat;

    size_t freed = shrink(snapshot);
    size_t copied = 0;

    if (m_compacting == gc_compacting::ENABLED) {
        if (threads_available > 1) {
            auto cmpct_res = parallel_compact(threads_available);
            forwarding& frwd = cmpct_res.first;
            copied = cmpct_res.second;
            parallel_fix_pointers(frwd, threads_available);
            snapshot.fix_roots(frwd);
        } else {
            auto cmpct_res = compact();
            forwarding& frwd = cmpct_res.first;
            copied = cmpct_res.second;
            fix_pointers(frwd);
            snapshot.fix_roots(frwd);
        }
    }

    freed += sweep();
//    unmark();

    stat.mem_swept = freed;
    stat.mem_copied = copied;

    return stat;
}

gc_pointer_type gc_heap::allocate_on_tlab(size_t size)
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

    size_t freed_in_tlabs = 0;
    for (auto it = m_tlab_map.begin(); it != m_tlab_map.end(); ) {
        freed_in_tlabs += it->second.shrink();
        if (it->second.empty() && snapshot.lookup_thread(it->first) == nullptr) {
            it = m_tlab_map.erase(it);
        } else {
            ++it;
        }
    }

    size_t freed_in_loa = m_loa.shrink();

    return freed_in_tlabs + freed_in_loa;
}

size_t gc_heap::sweep()
{
    size_t freed = 0;
    for (auto& kv: m_tlab_map) {
        kv.second.apply_to_chunks([&freed] (allocators::managed_pool_chunk& chunk) {
            freed += chunk.sweep();
            chunk.unmark();
        });
        kv.second.reset_cache();
    }
    return freed;
}

std::pair<gc_heap::forwarding, size_t> gc_heap::compact()
{
    logging::info() << "Compacting memory...";

    forwarding frwd;
    size_t mem_copied = 0;
    for (auto& kv: m_tlab_map) {
        auto& tlab = kv.second;
        for (size_t i = 0; i < tlab_bucket_policy::BUCKET_COUNT; ++i) {
            auto rng = tlab.memory_range(i);
            size_t bucket_size = tlab_bucket_policy::bucket_size(i);
            size_t copied_cnt = two_finger_compact(rng, bucket_size, frwd);
            mem_copied += copied_cnt * bucket_size;
        }
    }
    return std::make_pair(frwd, mem_copied);
}

std::pair<gc_heap::forwarding, size_t> gc_heap::parallel_compact(size_t threads_num)
{
    logging::info() << "Compacting memory (in parallel with " << threads_num << " threads)...";

    utils::static_thread_pool thread_pool(threads_num);
    std::vector<std::function<void()>> tasks;
    std::atomic<size_t> mem_copied{0};

    forwarding frwd;
    for (auto& kv: m_tlab_map) {
        auto& tlab = kv.second;
        for (size_t i = 0; i < tlab_bucket_policy::BUCKET_COUNT; ++i) {
            auto rng = tlab.memory_range(i);
            size_t bucket_size = tlab_bucket_policy::bucket_size(i);
            if (!rng.empty()) {
                tasks.emplace_back([this, rng, i, bucket_size, &mem_copied, &frwd] {
                    size_t copied_cnt = two_finger_compact(rng, bucket_size, frwd);
                    mem_copied += copied_cnt * bucket_size;
                });
            }
        }
    }
    thread_pool.run(tasks.begin(), tasks.end());
    return std::make_pair(frwd, mem_copied.load());
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

//void gc_heap::unmark()
//{
//    for (auto& kv: m_tlab_map) {
//        kv.second.apply_to_chunks([] (allocators::managed_pool_chunk& chunk) {
//            chunk.unmark();
//        });
//    }
//}

}}