#ifndef DIPLOMA_CONSTANTS_H
#define DIPLOMA_CONSTANTS_H

#include <cstddef>

namespace precisegc {namespace details {

const size_t SYSTEM_POINTER_BITS_COUNT = 64;
const size_t RESERVED_BITS_COUNT = 3;

const size_t MEMORY_CELL_SIZE_BITS = 12;
const size_t MEMORY_CELL_SIZE = 1 << MEMORY_CELL_SIZE_BITS;

}}

#endif //DIPLOMA_CONSTANTS_H
