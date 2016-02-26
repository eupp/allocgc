#ifndef DIPLOMA_CONSTANTS_H
#define DIPLOMA_CONSTANTS_H

#include <cstddef>

namespace precisegc { namespace details {

const size_t POINTER_BITS_CNT = 64;
const size_t RESERVED_BITS_CNT = 3;

const size_t PAGE_BITS_CNT = 12;
const size_t PAGE_SIZE = 1 << PAGE_BITS_CNT;

}}

#endif //DIPLOMA_CONSTANTS_H
