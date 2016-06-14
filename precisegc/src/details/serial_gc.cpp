#include <libprecisegc/details/serial_gc.hpp>

#include <utility>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_unsafe_scope.h>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>

namespace precisegc { namespace details {

serial_gc::serial_gc(gc_compacting compacting,
                     std::unique_ptr<initation_policy> init_policy)
    : m_initator(this, std::move(init_policy))
    , m_heap(compacting)
{}

managed_ptr serial_gc::allocate(size_t size)
{
    return m_heap.allocate(size);
}

byte* serial_gc::rbarrier(const atomic_byte_ptr& p)
{
    return p.load(std::memory_order_relaxed);
}

void serial_gc::wbarrier(atomic_byte_ptr& dst, const atomic_byte_ptr& src)
{
    gc_unsafe_scope unsafe_scope;
    dst.store(src.load(std::memory_order_relaxed), std::memory_order_relaxed);
}

void serial_gc::initation_point(initation_point_type ipoint)
{
    m_initator.initation_point(ipoint);
}

gc_info serial_gc::info() const
{
    static gc_info inf = {
        .incremental                = false,
        .support_concurrent_mark    = false,
        .support_concurrent_sweep   = false
    };

    return inf;
}

void serial_gc::gc()
{
    using namespace threads;
    world_snapshot snapshot = thread_manager::instance().stop_the_world();
    m_marker.trace_roots(snapshot);
    m_marker.trace_pins(snapshot);
    m_marker.mark();

    gc_sweep_stat sweep_stat = m_heap.sweep();
    gc_pause_stat pause_stat = {
            .type       = gc_pause_type::GC,
            .duration   = snapshot.time_since_stop_the_world()};

    gc_register_sweep(sweep_stat, pause_stat);
}

}}

