#include <liballocgc/details/allocators/gc_core_allocator.hpp>

#include <cstring>

namespace allocgc { namespace details { namespace allocators {

const size_t gc_core_allocator::HEAP_START_LIMIT = 4 * 1024 * 1024;

const double gc_core_allocator::INCREASE_FACTOR = 2.0;

const double gc_core_allocator::MARK_THRESHOLD = 0.6;

const double gc_core_allocator::COLLECT_THRESHOLD = 1.0;

gc_core_allocator::gc_core_allocator()
    : m_gc_launcher(nullptr)
    , m_heap_size(0)
    , m_heap_limit(HEAP_START_LIMIT)
    , m_heap_maxlimit(HEAP_START_LIMIT)
{ }

gc_core_allocator::gc_core_allocator(gc_launcher* gc)
    : m_gc_launcher(gc)
    , m_heap_size(0)
    , m_heap_limit(HEAP_START_LIMIT)
    , m_heap_maxlimit(HEAP_START_LIMIT)
{ }

byte* gc_core_allocator::allocate(size_t size, bool zeroing)
{
    assert(size != 0);
    size_t aligned_size = sys_allocator::align_size(size);

    std::unique_lock<mutex_t> lock(m_mutex);

    if (!increase_heap_size(aligned_size, &lock)) {
        return nullptr;
    }

    byte* page = nullptr;
    if (aligned_size <= MAX_BUCKETIZE_SIZE) {
        page = m_bucket_alloc.allocate(aligned_size);
        if (zeroing) {
            memset(page, 0, aligned_size);
        }
    } else {
        page = sys_allocator::allocate(aligned_size);
    }
    return page;
}

void gc_core_allocator::deallocate(byte* ptr, size_t size)
{
    assert(size != 0);

    std::lock_guard<mutex_t> lock(m_mutex);

    size_t aligned_size = sys_allocator::align_size(size);

    if (aligned_size <= MAX_BUCKETIZE_SIZE) {
        m_bucket_alloc.deallocate(ptr, aligned_size);
    } else {
        m_freelist.deallocate(ptr, aligned_size);
    }

    decrease_heap_size(aligned_size);
}

size_t gc_core_allocator::shrink()
{
    std::lock_guard<mutex_t> lock(m_mutex);
    return m_freelist.shrink();
}

gc_core_allocator::memory_range_type gc_core_allocator::memory_range()
{
    return memory_range_type(nullptr, nullptr);
}

bool gc_core_allocator::increase_heap_size(size_t alloc_size, std::unique_lock<mutex_t>* lock)
{
    size_t size = m_heap_size + alloc_size;
    if (size > COLLECT_THRESHOLD * m_heap_limit) {
        return false;
    }
    else if (size > MARK_THRESHOLD * m_heap_limit) {
        assert(m_gc_launcher);

        gc_options opt;
        opt.kind = gc_kind::CONCURRENT_MARK;
        opt.gen  = 0;

        lock->unlock();
        m_gc_launcher->gc(opt);
        lock->lock();
    }
    m_heap_size += alloc_size;
    return true;
}

void gc_core_allocator::decrease_heap_size(size_t size)
{
    m_heap_size -= size;
}

void gc_core_allocator::set_heap_limit(size_t limit)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_heap_maxlimit = limit;
}

void gc_core_allocator::expand_heap()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t increased_size = INCREASE_FACTOR * m_heap_limit;
    m_heap_limit = std::min(increased_size, m_heap_maxlimit);
}

gc_run_stat gc_core_allocator::gc(const gc_options& options)
{
    assert(m_gc_launcher);
    return m_gc_launcher->gc(options);
}

size_t gc_core_allocator::page_bucket_policy::bucket(size_t size)
{
    return (size >> PAGE_SIZE_LOG2) - 1;
}

size_t gc_core_allocator::page_bucket_policy::bucket_size(size_t i)
{
    return (i + 1) << PAGE_SIZE_LOG2;
}

}}}
