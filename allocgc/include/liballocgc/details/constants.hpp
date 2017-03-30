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

const size_t MANAGED_CHUNK_OBJECTS_COUNT = PAGE_SIZE / MIN_CELL_SIZE;


namespace stack_growth_direction{
enum stack_growth_direction_enum {
      UP = 1
    , DOWN = -1
};
}

const int STACK_DIRECTION = stack_growth_direction::DOWN;

}}

#endif //ALLOCGC_ALLOCATORS_CONSTANTS_H
