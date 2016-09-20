#include <libprecisegc/details/allocators/core_allocator.hpp>

#include <cstring>

namespace precisegc { namespace details { namespace allocators {

core_allocator::freelist_alloc_t core_allocator::freelist{};
core_allocator::bucket_alloc_t core_allocator::bucket_alloc{};
core_allocator::mutex_t core_allocator::mutex{};

byte* core_allocator::allocate(size_t size)
{
    assert(size != 0);
    size_t aligned_size = sys_allocator::align_size(size);
    gc_initiation_point(initiation_point_type::HEAP_EXPANSION,
                        initiation_point_data::create_heap_expansion_data(aligned_size));

    byte* page = nullptr;
    if (aligned_size <= MAX_BUCKETIZE_SIZE) {
        std::lock_guard<mutex_t> lock(mutex);
        page = bucket_alloc.allocate(aligned_size).decorated();
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
    size_t aligned_size = sys_allocator::align_size(size);
    gc_deregister_page(ptr, aligned_size);

    if (aligned_size <= MAX_BUCKETIZE_SIZE) {
        std::lock_guard<mutex_t> lock(mutex);
        bucket_alloc.deallocate(ptr, aligned_size);
    } else {
        freelist.deallocate(ptr, aligned_size);
    }
}

size_t core_allocator::shrink()
{
    return freelist.shrink();
}

core_allocator::memory_range_type core_allocator::memory_range()
{
    return memory_range_type(nullptr, nullptr);
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
