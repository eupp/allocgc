#include <libprecisegc/details/allocators/gc_core_allocator.hpp>

#include <cstring>

namespace precisegc { namespace details { namespace allocators {

gc_core_allocator::mutex_t gc_core_allocator::mutex{};
gc_core_allocator::freelist_alloc_t gc_core_allocator::freelist{};
gc_core_allocator::bucket_alloc_t gc_core_allocator::bucket_alloc{};

byte* gc_core_allocator::allocate(size_t size)
{
    assert(size != 0);
    size_t aligned_size = sys_allocator::align_size(size);

    if (!gc_increase_heap_size(aligned_size)) {
        return nullptr;
    }

    std::lock_guard<mutex_t> lock(mutex);

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

void gc_core_allocator::deallocate(byte* ptr, size_t size)
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

size_t gc_core_allocator::shrink()
{
    std::lock_guard<mutex_t> lock(mutex);
    return freelist.shrink();
}

gc_core_allocator::memory_range_type gc_core_allocator::memory_range()
{
    return memory_range_type(nullptr, nullptr);
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
