#include <libprecisegc/details/allocators/core_allocator.hpp>

namespace precisegc { namespace details { namespace allocators {

core_allocator::freelist_t core_allocator::freelist{};

byte* core_allocator::allocate(size_t size)
{
    assert(size != 0);
    size_t aligned_size = sys_allocator::align_size(size);
    gc_initiation_point(initiation_point_type::HEAP_EXPANSION,
                        initiation_point_data::create_heap_expansion_data(aligned_size));
    byte* page = sys_allocator::allocate(aligned_size);
    gc_register_page(page, aligned_size);

    return page;
}

void core_allocator::deallocate(byte* ptr, size_t size)
{
    assert(size != 0 && size % PAGE_SIZE == 0);
    gc_deregister_page(ptr, size);
    freelist.deallocate(ptr, size);
}

size_t core_allocator::shrink()
{
    return freelist.shrink();
}

core_allocator::memory_range_type core_allocator::memory_range()
{
    return memory_range_type(nullptr, nullptr);
}


}}}
