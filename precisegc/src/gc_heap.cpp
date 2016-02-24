#include "gc_heap.h"

#include <cassert>

#include "gc_compact.h"
#include "math_util.h"

namespace precisegc { namespace details {

gc_heap::gc_heap()
    : m_size(0)
{
    size_t alloc_size = MIN_ALLOC_SIZE_BITS;
    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i, ++alloc_size) {
        m_storage[i].set_alloc_size(1 << alloc_size);
    }
}

gc_heap::allocate_result gc_heap::allocate(size_t size)
{
    lock_guard<mutex> lock(m_mutex);
    size_t aligned_size = align_size(size);
    size_t sl_ind = log_2(aligned_size) - MIN_ALLOC_SIZE_BITS;
    assert(aligned_size == m_storage[sl_ind].alloc_size());
    auto alloc_res = m_storage[sl_ind].allocate();
    m_size += aligned_size;
    return std::make_pair(alloc_res.first, aligned_size);
}

size_t gc_heap::size() noexcept
{
    lock_guard<mutex> lock(m_mutex);
    return m_size;
}

void gc_heap::compact()
{
    // don't lock here, because compact is guarantee to be called during gc phase,
    // while other threads are suspended.
//    lock_guard<mutex> lock(m_mutex);
    intrusive_forwarding frwd = compact_memory();
    fix_pointers(frwd);
    fix_roots(frwd);
}

intrusive_forwarding gc_heap::compact_memory()
{
    intrusive_forwarding frwd;
    size_t compacted_size = 0;
    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
        compacted_size += two_finger_compact(m_storage[i].begin(), m_storage[i].end(), m_storage[i].alloc_size(), frwd);
        m_storage[i].clear_mark_bits();

//        logging::info() << "Already compacted " << compacted_size << " bytes";
    }
    assert(m_size >= compacted_size);
    m_size -= compacted_size;
    return frwd;
}

void gc_heap::fix_pointers(const intrusive_forwarding& frwd)
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