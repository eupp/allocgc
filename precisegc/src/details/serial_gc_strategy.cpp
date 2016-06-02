#include <libprecisegc/details/serial_gc_strategy.hpp>

#include <utility>

#include <libprecisegc/details/gc_unsafe_scope.h>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/world_state.hpp>

namespace precisegc { namespace details {

serial_gc_strategy::serial_gc_strategy(gc_compacting compacting,
                                                   std::unique_ptr<initation_policy> init_policy)
    : m_initator(this, std::move(init_policy))
    , m_heap(compacting)
{}

managed_ptr serial_gc_strategy::allocate(size_t size)
{
    return m_heap.allocate(size);
}

byte* serial_gc_strategy::rbarrier(const atomic_byte_ptr& p)
{
    return p.load(std::memory_order_relaxed);
}

void serial_gc_strategy::wbarrier(atomic_byte_ptr& dst, const atomic_byte_ptr& src)
{
    gc_unsafe_scope unsafe_scope;
    dst.store(src.load(std::memory_order_relaxed), std::memory_order_relaxed);
}

void serial_gc_strategy::initation_point(initation_point_type ipoint)
{
    m_initator.gc_initation_point(ipoint);
}

gc_info serial_gc_strategy::info() const
{
    static gc_info inf = {
        .incremental                = false,
        .support_concurrent_mark    = false,
        .support_concurrent_sweep   = false
    };

    return inf;
}

void serial_gc_strategy::gc()
{
    using namespace threads;
    world_state wstate = thread_manager::instance().stop_the_world();
    m_marker.trace_roots(wstate);
    m_marker.trace_pins(wstate);
    m_marker.mark();
    m_heap.sweep();
}

}}

