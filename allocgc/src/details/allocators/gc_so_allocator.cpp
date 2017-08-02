#include <liballocgc/details/allocators/gc_so_allocator.hpp>

#include <algorithm>

#include <liballocgc/details/allocators/memory_index.hpp>

namespace allocgc { namespace details { namespace allocators {

gc_so_allocator::gc_so_allocator(gc_core_allocator* core_alloc)
{
    for (size_t i = 0; i < m_bucket_policy.BUCKET_COUNT; ++i) {
        m_buckets[i].init(&m_bucket_policy, core_alloc);
    }
}

gc_alloc::response gc_so_allocator::allocate(const gc_alloc::request& rqst)
{
    size_t size = gc_box::box_size(rqst.alloc_size());
    assert(size <= LARGE_CELL_SIZE);
    size_t bucket_idx = m_bucket_policy.bucket_id(size);
    return m_buckets[bucket_idx].allocate(rqst, m_bucket_policy.sz_cls(bucket_idx));
}

gc_collect_stat gc_so_allocator::collect(compacting::forwarding& frwd, thread_pool_t& thread_pool)
{
    std::vector<std::function<void()>> tasks;
    std::array<gc_collect_stat, m_bucket_policy.BUCKET_COUNT> part_stats;
    for (size_t i = 0; i < m_bucket_policy.BUCKET_COUNT; ++i) {
        if (m_buckets[i].empty()) {
            continue;
        }
        tasks.emplace_back([this, i, &frwd, &part_stats] {
            part_stats[i] = m_buckets[i].collect(frwd);
        });
    }
    thread_pool.run(tasks.begin(), tasks.end());

    gc_collect_stat stat;
    for (auto& part_stat: part_stats) {
        stat += part_stat;
    }

    return stat;
}

void gc_so_allocator::fix(const compacting::forwarding& frwd, thread_pool_t& thread_pool)
{
    std::vector<std::function<void()>> tasks;
    for (size_t i = 0; i < m_bucket_policy.BUCKET_COUNT; ++i) {
        if (m_buckets[i].empty()) {
            continue;
        }
        tasks.emplace_back([this, i, &frwd] {
            m_buckets[i].fix(frwd);
        });
    }
    thread_pool.run(tasks.begin(), tasks.end());
}

void gc_so_allocator::finalize()
{
    for (size_t i = 0; i < m_bucket_policy.BUCKET_COUNT; ++i) {
        m_buckets[i].finalize();
    }
}

gc_memstat gc_so_allocator::stats()
{
    gc_memstat stat;
    for (auto& bucket: m_buckets) {
        stat += bucket.stats();
    }
    return stat;
}

}}}
