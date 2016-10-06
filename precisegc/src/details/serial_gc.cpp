#include <libprecisegc/details/collectors/serial_gc.hpp>

#include <stdexcept>
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
    , m_marker(&m_packet_manager, nullptr)
    , m_threads_available(threads_available)
{}

gc_alloc_descriptor serial_gc_base::allocate(size_t obj_size, size_t obj_cnt, const type_meta* tmeta)
{
    gc_alloc_descriptor descr = m_heap.allocate(sizeof(traceable_object_meta) + obj_size * obj_cnt);
    traceable_object_meta* meta = managed_object::get_meta(descr.get());
    new (meta) traceable_object_meta(obj_cnt, tmeta);
    return gc_alloc_descriptor(managed_object::get_object(descr.get()),
                               descr.size() - sizeof(traceable_object_meta),
                               descr.descriptor());
}

void serial_gc_base::commit(const gc_alloc_descriptor& ptr)
{
    return;
}

byte* serial_gc_base::rbarrier(const gc_word& handle)
{
    return dptr_storage::get(gc_handle_access::get<std::memory_order_relaxed>(handle));
}

void serial_gc_base::wbarrier(gc_word& dst, const gc_word& src)
{
    byte* p = gc_handle_access::get<std::memory_order_relaxed>(src);
    gc_handle_access::set<std::memory_order_relaxed>(dst, p);
}

void serial_gc_base::interior_wbarrier(gc_word& handle, ptrdiff_t offset)
{
    byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(handle);
    if (dptr_storage::is_derived(ptr)) {
        dptr_storage::reset_derived_ptr(ptr, offset);
    } else {
        byte* derived = m_dptr_storage.make_derived(ptr, offset);
        gc_handle_access::set<std::memory_order_relaxed>(handle, derived);
    }
}

gc_run_stats serial_gc_base::gc(const gc_options& options)
{
    if (options.phase != gc_phase::COLLECT) {
        throw std::invalid_argument("serial_gc supports only gc_phase::COLLECT option");
    }

    using namespace threads;
    world_snapshot snapshot = thread_manager::instance().stop_the_world();
    m_marker.trace_roots(snapshot.get_root_tracer());
    m_marker.trace_pins(snapshot.get_pin_tracer());
    m_marker.concurrent_mark(m_threads_available - 1);
    m_marker.mark();
    auto collect_stats = m_heap.collect(snapshot, m_threads_available);

    gc_run_stats stats = {
            .type           = gc_type::FULL_GC,
            .mem_swept      = collect_stats.mem_swept,
            .mem_copied     = collect_stats.mem_copied,
            .pause_duration = snapshot.time_since_stop_the_world()
    };

    return stats;
}

}

serial_gc::serial_gc(size_t threads_available)
    : serial_gc_base(gc_compacting::DISABLED, threads_available)
{}

gc_info serial_gc::info() const
{
    static gc_info inf = {
        .incremental_flag           = false,
        .support_concurrent_marking    = false,
        .support_concurrent_collecting   = false
    };
    return inf;
}

serial_compacting_gc::serial_compacting_gc(size_t threads_available)
    : serial_gc_base(gc_compacting::ENABLED, threads_available)
{}

void serial_compacting_gc::wbarrier(gc_word& dst, const gc_word& src)
{
    gc_unsafe_scope unsafe_scope;
    internals::serial_gc_base::wbarrier(dst, src);
}

void serial_compacting_gc::interior_wbarrier(gc_word& handle, ptrdiff_t offset)
{
    gc_unsafe_scope unsafe_scope;
    internals::serial_gc_base::interior_wbarrier(handle, offset);
}

gc_info serial_compacting_gc::info() const
{
    static gc_info inf = {
            .incremental_flag           = false,
            .support_concurrent_marking    = false,
            .support_concurrent_collecting   = false
    };
    return inf;
}

}}}

