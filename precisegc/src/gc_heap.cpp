#include "gc_heap.h"

#include <cassert>

#include "gc_compact.h"
#include "math_util.h"

namespace precisegc { namespace details {

gc_heap::gc_heap()
    : m_size(0)
{}

managed_cell_ptr gc_heap::allocate(size_t size)
{
    managed_cell_ptr cell_ptr = m_alloc.allocate(size);
    m_size.fetch_add(cell_ptr.cell_size());
    return std::move(cell_ptr);
}

size_t gc_heap::size() noexcept
{
    return m_size.load();
}

void gc_heap::compact()
{
    // don't lock here, because compact is guarantee to be called during gc phase,
    // while other threads are suspended.
//    lock_guard<mutex> lock(m_mutex);
    gc_heap::forwarding frwd = compact_memory();
    fix_pointers(frwd);
    fix_roots(frwd);
}

gc_heap::forwarding gc_heap::compact_memory()
{
    forwarding frwd;
    size_t sweep_size = 0;
    auto& bp = m_alloc.get_bucket_policy();
    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
        auto rng = m_alloc.range(i);
        two_finger_compact(rng, bp.bucket_size(i), frwd);
        size_t sweep_cnt = sweep(rng);
        sweep_size += sweep_cnt * bp.bucket_size(i);
    }
    assert(m_size >= sweep_size);
    m_size -= sweep_size;
    return frwd;
}

void gc_heap::fix_pointers(const gc_heap::forwarding& frwd)
{
    auto& bp = m_alloc.get_bucket_policy();
    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
        auto rng = m_alloc.range(i);
        ::precisegc::details::fix_pointers(rng.begin(), rng.end(), bp.bucket_size(i), frwd);
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