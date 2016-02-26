#ifndef DIPLOMA_MMAP_H
#define DIPLOMA_MMAP_H

#include <cstddef>
#include <cstdint>

#include "types.h"

namespace precisegc { namespace details { namespace allocators {

byte* memory_allocate(size_t size);
void memory_deallocate(byte* ptr, size_t size);

class native_allocator
{
public:
    byte* allocate(size_t);
    void deallocate(byte* ptr, size_t size);
};

}}}

#endif //DIPLOMA_MMAP_H
