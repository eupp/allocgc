#include <libprecisegc/details/allocators/mso_allocator.hpp>

#include <algorithm>

#include <libprecisegc/details/collectors/memory_index.hpp>

namespace precisegc { namespace details { namespace allocators {

mso_allocator::mso_allocator()
{
    for (size_t i = 0; i < BUCKET_COUNT; i += 2) {
        size_t sz_cls = 1ull << (i + MIN_CELL_SIZE);
        m_buckets[i].first = sz_cls;
        if (i + 1 < BUCKET_COUNT) {
            m_buckets[i + 1].first = sz_cls + sz_cls / 2;
        }
    }
}

gc_alloc_response mso_allocator::allocate(const gc_alloc_request& rqst)
{
    auto it = std::lower_bound(m_buckets.begin(), m_buckets.end(), rqst.alloc_size(),
                               [] (const bucket_t& a, const bucket_t& b) { return a.first < b.first; });
    return it->second.allocate(rqst, it->first);
}

gc_heap_stat mso_allocator::collect(compacting::forwarding& frwd, thread_pool_t& thread_pool)
{
    std::vector<std::function<void()>> tasks;
    std::array<gc_heap_stat, BUCKET_COUNT> part_stats;
    for (size_t i = 0; i < BUCKET_COUNT; ++i) {
        if (m_buckets[i].second.empty()) {
            continue;
        }
        tasks.emplace_back([this, i, &frwd, &part_stats] {
            part_stats[i] = m_buckets[i].second.collect(frwd);
        });
    }
    thread_pool.run(tasks.begin(), tasks.end());

    gc_heap_stat stat;
    for (auto& part_stat: part_stats) {
        stat.mem_used       += part_stat.mem_used;
        stat.mem_freed      += part_stat.mem_freed;
        stat.mem_copied     += part_stat.mem_copied;
        stat.mem_residency  += part_stat.mem_residency;
        stat.pinned_cnt     += part_stat.pinned_cnt;
    }
    stat.mem_residency /= part_stats.size();

    return stat;
}

void mso_allocator::fix(const compacting::forwarding& frwd, thread_pool_t& thread_pool)
{
    std::vector<std::function<void()>> tasks;
    for (size_t i = 0; i < BUCKET_COUNT; ++i) {
        if (m_buckets[i].second.empty()) {
            continue;
        }
        tasks.emplace_back([this, i, &frwd] {
            m_buckets[i].second.fix(frwd);
        });
    }
    thread_pool.run(tasks.begin(), tasks.end());
}

}}}
