#include "gc_heap.h"

#include <cassert>

#include "gc_compact.h"
#include "math_util.h"

namespace precisegc { namespace details {

gc_heap::gc_heap()
{
    size_t alloc_size = MIN_ALLOC_SIZE_BITS;
    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i, ++alloc_size) {
        m_storage[i].set_alloc_size(1 << alloc_size);
    }
}

object_meta* gc_heap::allocate(size_t obj_size, size_t count, const class_meta* cls_meta)
{
    lock_guard<mutex> lock(m_mutex);
    size_t size = obj_size * count + sizeof(object_meta);
    size_t aligned_size = align_size(size);
    size_t sl_ind = log_2(aligned_size) - MIN_ALLOC_SIZE_BITS;
    assert(aligned_size == m_storage[sl_ind].alloc_size());
    auto alloc_res = m_storage[sl_ind].allocate();
    void* ptr = alloc_res.first;
    return new (object_meta::get_meta(ptr, aligned_size)) object_meta((const class_meta*) cls_meta, count, ptr);
}

void gc_heap::compact()
{
    // don't lock here, because compact is guarantee to be called during gc phase,
    // while other threads are suspended.
//    lock_guard<mutex> lock(m_mutex);
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
        return size;
    }
    // otherwise --- round it up
    while (i != 0) {
        size = i;
        i = size & (size -1);
    }
    size = size << 1;
    return size;
}

}}