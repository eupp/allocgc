#ifndef DIPLOMA_MMAP_H
#define DIPLOMA_MMAP_H

#include <cstddef>
#include <cstdint>

#include "types.h"
#include "memory.h"


namespace precisegc { namespace details { namespace allocators {

class paged_allocator
{
public:
    typedef byte* pointer_type;

    paged_allocator() = default;
    paged_allocator(const paged_allocator&) = default;
    paged_allocator(paged_allocator&&) = default;

    byte* allocate(size_t size)
    {
        return paged_memory_allocate(size);
    }

    void deallocate(byte* ptr, size_t size)
    {
        paged_memory_deallocate(ptr, size);
    }
};

}}}

#endif //DIPLOMA_MMAP_H
