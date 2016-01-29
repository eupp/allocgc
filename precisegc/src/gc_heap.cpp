#include "gc_heap.h"

#include <cassert>

#include "gc_compact.h"
#include "util.h"

namespace precisegc { namespace details {

gc_heap::gc_heap()
{
    size_t alloc_size = MIN_ALLOC_SIZE_BITS;
    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i, ++alloc_size) {
        m_storage[i].set_alloc_size(1 << alloc_size);
    }
}

Object* gc_heap::allocate(size_t obj_size, size_t count, void* cls_meta)
{
    mutex_lock<mutex> lock(m_mutex);
    size_t size = obj_size * count + sizeof(Object);
    size_t aligned_size = align_size(size);
    size_t sl_ind = log_2(aligned_size) - MIN_ALLOC_SIZE_BITS;
    assert(aligned_size == m_storage[sl_ind].alloc_size());
    auto alloc_res = m_storage[sl_ind].allocate();
    void* ptr = alloc_res.first;
    Object* obj = (Object *) (ptr + aligned_size - sizeof(Object));
    obj->meta  = cls_meta;
    obj->count = count;
    obj->begin = ptr;
    return obj;
}

void gc_heap::compact()
{
    mutex_lock<mutex> lock(m_mutex);
    forwarding_list frwd = compact_memory();
    fix_pointers(frwd);
    fix_roots(frwd);
}

forwarding_list gc_heap::compact_memory()
{
    forwarding_list frwd;
    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
        two_finger_compact(m_storage[i].begin(), m_storage[i].end(), m_storage[i].alloc_size(), frwd);
        m_storage[i].clear_mark_bits();
    }
    return frwd;
}

void gc_heap::fix_pointers(const forwarding_list &frwd)
{
    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
        ::precisegc::details::fix_pointers(m_storage[i].begin(), m_storage[i].end(), m_storage[i].alloc_size(), frwd);
    }
}

size_t gc_heap::align_size(size_t size)
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