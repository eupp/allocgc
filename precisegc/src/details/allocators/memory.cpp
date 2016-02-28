#include "allocators/memory.h"

#include <cstring>
#include <sys/mman.h>

#include "logging.h"

namespace precisegc { namespace details { namespace allocators {

byte* paged_memory_allocate(size_t size)
{
    void* mem = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (mem == MAP_FAILED) {
        logging::debug() << "mmap failed: " << strerror(errno);
        assert(false);
    }
    return reinterpret_cast<byte*>(mem);
}

void paged_memory_deallocate(byte* ptr, size_t size)
{
    if (!ptr) {
        return;
    }
    int ret = munmap(reinterpret_cast<void*>(ptr), size);
    if (ret == -1) {
        logging::debug() << "munmap failed: " << strerror(errno);
    }
}

}}}