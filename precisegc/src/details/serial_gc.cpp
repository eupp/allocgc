#include <libprecisegc/details/collectors/serial_gc.hpp>

#include <utility>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/threads/this_managed_thread.hpp>

namespace precisegc { namespace details { namespace collectors {

namespace internals {

serial_gc_base::serial_gc_base(gc_compacting compacting, size_t threads_available)
    : m_heap(compacting)
    , m_marker(&m_packet_manager)
    , m_threads_available(threads_available)
{}

managed_ptr serial_gc_base::allocate(size_t size)
{
    return m_heap.allocate(size);
}

byte* serial_gc_base::rbarrier(const gc_handle& handle)
{
    return gc_handle_access::load(handle, std::memory_order_relaxed);
}

void serial_gc_base::interior_wbarrier(gc_handle& handle, byte* ptr)
{
    gc_handle_access::store(handle, ptr, std::memory_order_relaxed);
}

void serial_gc_base::interior_shift(gc_handle& handle, ptrdiff_t shift)
{
    gc_handle_access::fetch_advance(handle, shift, std::memory_order_relaxed);
}

void serial_gc_base::gc(gc_phase phase)
{
    if (phase != gc_phase::SWEEP) {
        return;
    }

    using namespace threads;
    world_snapshot snapshot = thread_manager::instance().stop_the_world();
    m_marker.trace_roots(snapshot.get_root_tracer());
    m_marker.trace_pins(snapshot.get_pin_tracer());
    m_marker.concurrent_mark(m_threads_available - 1);
    m_marker.mark();

    gc_sweep_stat sweep_stat = m_heap.sweep(snapshot, m_threads_available);
    gc_pause_stat pause_stat = {
            .type       = gc_pause_type::GC,
            .duration   = snapshot.time_since_stop_the_world()
    };

    gc_register_sweep(sweep_stat, pause_stat);
}

}

serial_gc::serial_gc(size_t threads_available)
    : serial_gc_base(gc_compacting::DISABLED, threads_available)
{}

void serial_gc::wbarrier(gc_handle& dst, const gc_handle& src)
{
    byte* p = gc_handle_access::load(src, std::memory_order_relaxed);
    gc_handle_access::store(dst, p, std::memory_order_relaxed);
}

bool serial_gc::compare(const gc_handle& a, const gc_handle& b)
{
    return gc_handle_access::load(a, std::memory_order_relaxed) == gc_handle_access::load(b, std::memory_order_relaxed);
}

byte* serial_gc::pin(const gc_handle& handle)
{
    return gc_handle_access::load(handle, std::memory_order_relaxed);
}

void serial_gc::unpin(byte* ptr)
{
    return;
}

gc_info serial_gc::info() const
{
    static gc_info inf = {
        .incremental_flag           = false,
        .support_concurrent_mark    = false,
        .support_concurrent_sweep   = false
    };
    return inf;
}

serial_compacting_gc::serial_compacting_gc(size_t threads_available)
    : serial_gc_base(gc_compacting::ENABLED, threads_available)
{}

void serial_compacting_gc::wbarrier(gc_handle& dst, const gc_handle& src)
{
    gc_unsafe_scope unsafe_scope;
    byte* p = gc_handle_access::load(src, std::memory_order_relaxed);
    gc_handle_access::store(dst, p, std::memory_order_relaxed);
}

bool serial_compacting_gc::compare(const gc_handle& a, const gc_handle& b)
{
    gc_unsafe_scope unsafe_scope;
    return gc_handle_access::load(a, std::memory_order_relaxed) == gc_handle_access::load(b, std::memory_order_relaxed);
}

byte* serial_compacting_gc::pin(const gc_handle& handle)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = gc_handle_access::load(handle, std::memory_order_relaxed);
    if (ptr) {
        threads::this_managed_thread::pin(ptr);
    }
    return ptr;
}

void serial_compacting_gc::unpin(byte* ptr)
{
    threads::this_managed_thread::unpin(ptr);
}

gc_info serial_compacting_gc::info() const
{
    static gc_info inf = {
            .incremental_flag           = false,
            .support_concurrent_mark    = false,
            .support_concurrent_sweep   = false
    };
    return inf;
}

}}}

