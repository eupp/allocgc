#ifndef ALLOCGC_ALLOCATORS_CONSTANTS_H
#define ALLOCGC_ALLOCATORS_CONSTANTS_H

#include <cstddef>

namespace allocgc { namespace details {

const size_t POINTER_BITS_CNT   = 64;
const size_t POINTER_BITS_USED  = 48;
const size_t ALIGN_BITS_CNT     = 3;

const size_t PAGE_SIZE_LOG2  = 12;
const size_t PAGE_SIZE       = 1 << PAGE_SIZE_LOG2;

const size_t MIN_CELL_SIZE_LOG2 = 5;
const size_t MIN_CELL_SIZE      = 1 << MIN_CELL_SIZE_LOG2;

const size_t LARGE_CELL_SIZE_LOG2 = 12;
const size_t LARGE_CELL_SIZE      = 1 << LARGE_CELL_SIZE_LOG2;

const size_t GC_POOL_CHUNK_MAXSIZE = 16384;

const size_t GC_POOL_CHUNK_OBJECTS_COUNT = PAGE_SIZE / MIN_CELL_SIZE;

const size_t GRANULE_SIZE_LOG2 = 4;
const size_t GRANULE_SIZE = 1 << GRANULE_SIZE_LOG2;

}}

#endif //ALLOCGC_ALLOCATORS_CONSTANTS_H
