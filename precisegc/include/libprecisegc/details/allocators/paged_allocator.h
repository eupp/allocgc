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
