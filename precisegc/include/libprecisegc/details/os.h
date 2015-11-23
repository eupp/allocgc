#ifndef DIPLOMA_OS_H
#define DIPLOMA_OS_H

#include <cassert>
#include <sys/mman.h>
#include <malloc.h>

namespace precisegc { namespace details {

inline void* memory_allocate(size_t size)
{
    void* mem = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        mem = nullptr;
    }
    return mem;
}

inline void* memory_align_allocate(size_t align, size_t size)
{
    return memalign(align, size);
}

inline void memory_align_deallocate(void* ptr)
{
    free(ptr);
}

inline void memory_deallocate(void* ptr, size_t size)
{
    assert(ptr);
    munmap(ptr, size);
}

} }

#endif //DIPLOMA_OS_H
