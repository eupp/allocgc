#include "gc_heap.h"

#include <cassert>

#include "gc_compact.h"
#include "libprecisegc/details/utils/math.h"
#include "logging.h"

namespace precisegc { namespace details {

gc_heap::gc_heap(gc_compacting compacting)
    : m_compacting(compacting)
{}

managed_ptr gc_heap::allocate(size_t size)
{
    return m_alloc.allocate(size);
}

gc_sweep_stat gc_heap::sweep()
{
    gc_sweep_stat stat;

    logging::info() << "Shrinking memory...";
    stat.shrunk = m_alloc.shrink();


    if (m_compacting == gc_compacting::ENABLED) {
        logging::info() << "Compacting memory...";
        gc_heap::forwarding frwd = compact_memory();
        logging::info() << "Fixing pointers...";
        fix_pointers(frwd);
        logging::info() << "Fixing roots...";
        fix_roots(frwd);
    }

    m_alloc.apply_to_chunks([] (managed_pool_chunk& chunk) {
        chunk.unmark();
    });

    return stat;
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