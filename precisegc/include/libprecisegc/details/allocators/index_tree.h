#ifndef DIPLOMA_INDEX_TREE_H
#define DIPLOMA_INDEX_TREE_H

#include <cstddef>
#include <array>
#include <type_traits>

#include "constants.h"
#include "types.h"

namespace precisegc { namespace details { namespace allocators {

template <typename T, typename Alloc>
class index_tree
{
public:
    void index(byte* mem, T* entry)
    {

    }

    void remove_index(byte* mem)
    {

    }

    T* get_entry(byte* mem)
    {

    }
private:
    static const size_t LEVEL_CNT = 3;
    static const size_t USED_BITS_CNT = POINTER_BITS_CNT - PAGE_BITS_CNT;
    static const size_t LEVEL_BITS_CNT = USED_BITS_CNT / LEVEL_CNT;
    static const size_t FIRST_LEVEL_BITS_CNT = USED_BITS_CNT -  LEVEL_BITS_CNT * (LEVEL_CNT - 1);
    static const size_t LEVEL_SIZE = 1 << LEVEL_BITS_CNT;
    static const size_t FIRST_LEVEL_SIZE = 1 << FIRST_LEVEL_BITS_CNT;

    void* m_first_level[FIRST_LEVEL_SIZE];
    Alloc m_allocator;
};

}}}

#endif //DIPLOMA_INDEX_TREE_H
