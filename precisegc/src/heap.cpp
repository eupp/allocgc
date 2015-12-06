#include "details/heap.h"

#include <cassert>

#include "util.h"

namespace precisegc { namespace details {

heap::heap()
{
    size_t alloc_size = MIN_ALLOC_SIZE_BITS;
    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i, ++alloc_size) {
        m_storage[i].set_alloc_size(1 << alloc_size);
    }
}

void* heap::allocate(size_t size)
{
    mutex_lock<mutex> lock(&m_mutex);
    size_t aligned_size = align_size(size);
    size_t sl_ind = log_2(aligned_size) - MIN_ALLOC_SIZE_BITS;
    assert(aligned_size == m_storage[sl_ind].alloc_size());
    return m_storage[sl_ind].allocate().first;
}

forwarding_list heap::compact()
{
    mutex_lock<mutex> lock(&m_mutex);
    forwarding_list frwd;
    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
        m_storage[i].compact(frwd);
    }
    return frwd;
}

void heap::fix_pointers(const forwarding_list &forwarding)
{
    mutex_lock<mutex> lock(&m_mutex);
    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
        m_storage[i].fix_pointers(forwarding);
    }
}

size_t heap::align_size(size_t size)
{
    assert(size > 0);
    size_t i = size & (size -1);
    // if size === power of two then return it
    if (i == 0) {
        return size > MEMORY_CELL_SIZE ? size : MEMORY_CELL_SIZE;
    }
    // otherwise --- round it up
    while (i != 0) {
        size = i;
        i = size & (size -1);
    }
    size = size << 1;
    return size > MEMORY_CELL_SIZE ? size : MEMORY_CELL_SIZE;
}

}}