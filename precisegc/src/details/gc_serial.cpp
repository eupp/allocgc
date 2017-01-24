#include <libprecisegc/details/collectors/gc_serial.hpp>

#include <stdexcept>
#include <utility>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/collectors/gc_tagging.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/threads/this_managed_thread.hpp>

namespace precisegc { namespace details { namespace collectors {

gc_serial::gc_serial(size_t threads_available)
    : m_marker(&m_packet_manager, nullptr)
    , m_threads_available(threads_available)
{}

allocators::gc_alloc_response gc_serial::allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta)
{
    return m_heap.allocate(allocators::gc_alloc_request(obj_size, obj_cnt, tmeta));
}

void gc_serial::commit(gc_cell& cell)
{
    cell.commit();
}

void gc_serial::commit(gc_cell& cell, const gc_type_meta* type_meta)
{
    cell.commit(type_meta);
}

byte* gc_serial::init_ptr(byte* ptr, bool root_flag)
{
    return gc_tagging::set_root_bit(ptr, root_flag);
}

bool gc_serial::is_root(const gc_handle& handle) const
{
    return gc_tagging::is_root(gc_handle_access::get<std::memory_order_relaxed>(handle));
}

byte* gc_serial::rbarrier(const gc_handle& handle)
{
    return gc_tagging::get(gc_handle_access::get<std::memory_order_relaxed>(handle));
}

void gc_serial::wbarrier(gc_handle& dst, const gc_handle& src)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = gc_tagging::clear_root_bit(gc_handle_access::get<std::memory_order_relaxed>(src));
    bool root_bit = gc_tagging::is_root(gc_handle_access::get<std::memory_order_relaxed>(dst));
    gc_handle_access::set<std::memory_order_relaxed>(dst, gc_tagging::set_root_bit(ptr, root_bit));
}

void gc_serial::interior_wbarrier(gc_handle& handle, ptrdiff_t offset)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(handle);
    if (gc_tagging::is_derived(ptr)) {
        gc_tagging::reset_derived_ptr(ptr, offset);
    } else {
        dptr_descriptor* descr = m_dptr_storage.make_derived(gc_tagging::clear_root_bit(ptr), offset);
        byte* dptr = gc_tagging::make_derived_ptr(descr, gc_tagging::is_root(ptr));
        gc_handle_access::set<std::memory_order_relaxed>(handle, dptr);
    }
}

gc_run_stats gc_serial::gc(const gc_options& options)
{
    if ((options.kind != gc_kind::MARK_COLLECT) && (options.kind != gc_kind::COLLECT)) {
        return gc_run_stats();
    }

    gc_run_stats stats = sweep(options);
    allocators::gc_core_allocator::shrink();
    return stats;
}

gc_run_stats gc_serial::sweep(const gc_options& options)
{
    using namespace threads;

    world_snapshot snapshot = thread_manager::instance().stop_the_world();
    m_marker.trace_roots(snapshot.get_root_tracer());
    m_marker.trace_pins(snapshot.get_pin_tracer());
    m_marker.concurrent_mark(m_threads_available - 1);
    m_marker.mark();
    m_dptr_storage.destroy_unmarked();

    gc_run_stats stats;
    stats.gen = options.gen;
    stats.heap_stat = m_heap.collect(snapshot, options.gen, m_threads_available);

    stats.pause_stat.type       = gc_pause_type::MARK_COLLECT;
    stats.pause_stat.duration   = snapshot.time_since_stop_the_world();

    return stats;
}

gc_info gc_serial::info() const
{
    static gc_info inf = {
        .incremental_flag                = false,
        .support_concurrent_marking      = false,
        .support_concurrent_collecting   = false
    };
    return inf;
}

}}}

