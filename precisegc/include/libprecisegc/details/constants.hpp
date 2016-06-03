#ifndef DIPLOMA_ALLOCATORS_CONSTANTS_H
#define DIPLOMA_ALLOCATORS_CONSTANTS_H

#include <cstddef>

namespace precisegc { namespace details {

const size_t POINTER_BITS_CNT   = 64;
const size_t POINTER_BITS_USED  = 48;
const size_t ALIGN_BITS_CNT     = 3;


const size_t PAGE_BITS_CNT  = 12;
const size_t PAGE_SIZE      = 1 << PAGE_BITS_CNT;

const size_t MIN_CELL_SIZE  = 32;

}}

#endif //DIPLOMA_ALLOCATORS_CONSTANTS_H
