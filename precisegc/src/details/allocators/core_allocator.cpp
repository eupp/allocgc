#include <libprecisegc/details/allocators/core_allocator.hpp>

#include <cstring>

namespace precisegc { namespace details { namespace allocators {

const size_t core_allocator::HEAP_START_LIMIT = 2 * 1024 * 1024;

const double core_allocator::INCREASE_FACTOR      = 2.0;
const double core_allocator::MARK_THRESHOLD       = 0.6;
const double core_allocator::COLLECT_THRESHOLD    = 1.0;

size_t core_allocator::heap_limit = 0;
size_t core_allocator::heap_size = 0;
size_t core_allocator::heap_maxlimit = 0;

core_allocator::mutex_t core_allocator::mutex{};
core_allocator::freelist_alloc_t core_allocator::freelist{};
core_allocator::bucket_alloc_t core_allocator::bucket_alloc{};

byte* core_allocator::allocate(size_t size)
{
    assert(size != 0);
    size_t aligned_size = sys_allocator::align_size(size);

    std::lock_guard<mutex_t> lock(mutex);

    if (!check_heap_size(aligned_size)) {
        return nullptr;
    }

    byte* page = nullptr;
    if (aligned_size <= MAX_BUCKETIZE_SIZE) {
        page = bucket_alloc.allocate(aligned_size);
        memset(page, 0, aligned_size);
    } else {
        page = sys_allocator::allocate(aligned_size);
    }
    gc_register_page(page, aligned_size);
    return page;
}

void core_allocator::deallocate(byte* ptr, size_t size)
{
    assert(size != 0);

    std::lock_guard<mutex_t> lock(mutex);

    size_t aligned_size = sys_allocator::align_size(size);
    gc_deregister_page(ptr, aligned_size);

    if (aligned_size <= MAX_BUCKETIZE_SIZE) {
        bucket_alloc.deallocate(ptr, aligned_size);
    } else {
        freelist.deallocate(ptr, aligned_size);
    }
}

size_t core_allocator::shrink()
{
    std::lock_guard<mutex_t> lock(mutex);
    return freelist.shrink();
}

core_allocator::memory_range_type core_allocator::memory_range()
{
    return memory_range_type(nullptr, nullptr);
}

void core_allocator::expand_heap()
{
    std::lock_guard<mutex_t> lock(mutex);
    size_t increased_size = INCREASE_FACTOR * heap_limit;
    heap_limit = std::min(increased_size, heap_maxlimit);
}

void core_allocator::set_heap_limit(size_t size)
{
    std::lock_guard<mutex_t> lock(mutex);
    heap_limit = size == std::numeric_limits<size_t>::max() ? HEAP_START_LIMIT : size;
    heap_maxlimit = size;
}

bool core_allocator::check_heap_size(size_t alloc_size)
{
    size_t size = heap_size + alloc_size;
    if (size > COLLECT_THRESHOLD * heap_limit) {
        return false;
    }
    else if (size > MARK_THRESHOLD * heap_limit) {
        gc_options opt;
        opt.kind = gc_kind::CONCURRENT_MARK;
        opt.gen  = 0;
        gc_initiation_point(initiation_point_type::HEAP_LIMIT_EXCEEDED, opt);
    }
    heap_size += alloc_size;
    return true;
}

size_t core_allocator::page_bucket_policy::bucket(size_t size)
{
    return (size >> PAGE_BITS_CNT) - 1;
}

size_t core_allocator::page_bucket_policy::bucket_size(size_t i)
{
    return (i + 1) << PAGE_BITS_CNT;
}

}}}
