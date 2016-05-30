#include "gc_heap.h"

#include <cassert>

#include "gc_compact.h"
#include "libprecisegc/details/utils/math.h"
#include "logging.h"

namespace precisegc { namespace details {

gc_heap::gc_heap(gc_compacting compacting)
    : m_size(0)
    , m_compacting(compacting)
{}

managed_ptr gc_heap::allocate(size_t size)
{
    managed_ptr p = m_alloc.allocate(size);
    m_size.fetch_add(p.cell_size());
    return p;
}

size_t gc_heap::size() const noexcept
{
    return m_size.load();
}

void gc_heap::sweep()
{
    size_t shrinked_size = m_alloc.shrink();
    logging::info() << "Shrinked " << shrinked_size << " bytes";
    size_t freed = shrinked_size;
    assert(m_size >= freed);
    m_size.fetch_sub(freed);

    if (m_compacting == gc_compacting::ENABLED) {
        logging::info() << "Compacting memory...";
        gc_heap::forwarding frwd = compact_memory();
        logging::info() << "Fixing pointers...";
        fix_pointers(frwd);
        logging::info() << "Fixing roots...";
        fix_roots(frwd);
    }
//    logging::info() << "Sweeping...";
//    sweep();

    m_alloc.apply_to_chunks([] (managed_pool_chunk& chunk) {
        chunk.unmark();
    });
}

gc_heap::forwarding gc_heap::compact_memory()
{
    forwarding frwd;
    auto& bp = m_alloc.get_bucket_policy();
    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
        auto rng = m_alloc.memory_range(i);
        two_finger_compact(rng, bp.bucket_size(i), frwd);
    }
    return frwd;
}

void gc_heap::fix_pointers(const gc_heap::forwarding& frwd)
{
    auto& bp = m_alloc.get_bucket_policy();
    for (size_t i = 0; i < SEGREGATED_STORAGE_SIZE; ++i) {
        auto rng = m_alloc.memory_range(i);
        ::precisegc::details::fix_pointers(rng.begin(), rng.end(), bp.bucket_size(i), frwd);
    }
}

}}