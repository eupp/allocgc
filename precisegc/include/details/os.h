#ifndef DIPLOMA_OS_H
#define DIPLOMA_OS_H

#include <sys/mman.h>

namespace precisegc { namespace details {

void* memory_allocate(size_t size)
{
    void* mem = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        mem = nullptr;
    }
    return mem;
}

} }

#endif //DIPLOMA_OS_H
