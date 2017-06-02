#include <liballocgc/details/allocators/gc_so_allocator.hpp>

#include <algorithm>

#include <liballocgc/details/allocators/memory_index.hpp>

namespace allocgc { namespace details { namespace allocators {

size_t gc_so_allocator::SZ_CLS[] = {32, 64, 128, 256, 512, 1024, 2048, 4096};

gc_so_allocator::gc_so_allocator(gc_core_allocator* core_alloc)
{
    size_t j = 0;
    for (size_t i = 0; i < BUCKET_COUNT; ++i) {
        m_buckets[i].set_core_allocator(core_alloc);

        size_t sz_cls = SZ_CLS[i];
        while (j < sz_cls) {
            m_sztbl[j++] = i;
        }
    }
}

gc_alloc::response gc_so_allocator::allocate(const gc_alloc::request& rqst)
{
    size_t size = gc_box::box_size(rqst.alloc_size());
    assert(size <= LARGE_CELL_SIZE);
    size_t bucket_idx = m_sztbl[size - 1];
    return m_buckets[bucket_idx].allocate(rqst, SZ_CLS[bucket_idx]);
}

gc_heap_stat gc_so_allocator::collect(compacting::forwarding& frwd, thread_pool_t& thread_pool)
{
    std::vector<std::function<void()>> tasks;
    std::array<gc_heap_stat, BUCKET_COUNT> part_stats;
    for (size_t i = 0; i < BUCKET_COUNT; ++i) {
        if (m_buckets[i].empty()) {
            continue;
        }
        tasks.emplace_back([this, i, &frwd, &part_stats] {
            part_stats[i] = m_buckets[i].collect(frwd);
        });
    }
    thread_pool.run(tasks.begin(), tasks.end());

    gc_heap_stat stat;
    for (auto& part_stat: part_stats) {
        stat += part_stat;
    }

    return stat;
}

void gc_so_allocator::fix(const compacting::forwarding& frwd, thread_pool_t& thread_pool)
{
    std::vector<std::function<void()>> tasks;
    for (size_t i = 0; i < BUCKET_COUNT; ++i) {
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
    for (size_t i = 0; i < BUCKET_COUNT; ++i) {
        m_buckets[i].finalize();
    }
}

}}}
