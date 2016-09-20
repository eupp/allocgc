#ifndef DIPLOMA_ALLOCATORS_CONSTANTS_H
#define DIPLOMA_ALLOCATORS_CONSTANTS_H

#include <cstddef>

namespace precisegc { namespace details {

const size_t POINTER_BITS_CNT   = 64;
const size_t POINTER_BITS_USED  = 48;
const size_t ALIGN_BITS_CNT     = 3;


const size_t PAGE_BITS_CNT  = 12;
const size_t PAGE_SIZE      = 1 << PAGE_BITS_CNT;

const size_t MIN_CELL_SIZE_BITS_CNT = 5;
const size_t MIN_CELL_SIZE          = 1 << MIN_CELL_SIZE_BITS_CNT;

const size_t LARGE_CELL_SIZE_BITS_CNT = 12;
const size_t LARGE_CELL_SIZE          = 1 << LARGE_CELL_SIZE_BITS_CNT;

const size_t MANAGED_CHUNK_OBJECTS_COUNT = PAGE_SIZE / MIN_CELL_SIZE;

}}

#endif //DIPLOMA_ALLOCATORS_CONSTANTS_H
