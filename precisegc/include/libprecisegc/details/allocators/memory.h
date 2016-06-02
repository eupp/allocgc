#ifndef DIPLOMA_MEMORY_H
#define DIPLOMA_MEMORY_H

#include <cstddef>

#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace allocators {

byte* paged_memory_allocate(size_t size);
void paged_memory_deallocate(byte* ptr, size_t size);

}}}

#endif //DIPLOMA_MEMORY_H
