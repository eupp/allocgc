#include <libprecisegc/details/collectors/gc_incremental.hpp>

#include <cassert>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/threads/this_managed_thread.hpp>
#include <libprecisegc/details/collectors/memory_index.hpp>
#include <libprecisegc/details/collectors/gc_tagging.hpp>

namespace precisegc { namespace details { namespace collectors {

gc_incremental::gc_incremental(size_t threads_available)
    : m_marker(&m_packet_manager, &m_remset)
    , m_phase(gc_phase::IDLE)
    , m_threads_available(threads_available)
{}

allocators::gc_alloc_response gc_incremental::allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta)
{
    return m_heap.allocate(allocators::gc_alloc_request(obj_size, obj_cnt, tmeta));
}

void gc_incremental::commit(gc_cell& cell)
{
    cell.commit();
    if (m_phase == gc_phase::MARK) {
        cell.set_mark(true);
    }
}

void gc_incremental::commit(gc_cell& cell, const gc_type_meta* type_meta)
{
    cell.commit(type_meta);
    if (m_phase == gc_phase::MARK) {
        cell.set_mark(true);
    }
}

byte* gc_incremental::init_ptr(byte* ptr, bool root_flag)
{
    return gc_tagging::set_root_bit(ptr, root_flag);
}

bool gc_incremental::is_root(const gc_handle& handle) const
{
    return gc_tagging::is_root(gc_handle_access::get<std::memory_order_relaxed>(handle));
}

byte* gc_incremental::rbarrier(const gc_handle& handle)
{
    return gc_tagging::get(gc_handle_access::get<std::memory_order_relaxed>(handle));
}

void gc_incremental::wbarrier(gc_handle& dst, const gc_handle& src)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = gc_tagging::clear_root_bit(gc_handle_access::get<std::memory_order_relaxed>(src));
    bool root_bit = gc_tagging::is_root(gc_handle_access::get<std::memory_order_relaxed>(dst));
    gc_handle_access::set<std::memory_order_release>(dst, gc_tagging::set_root_bit(ptr, root_bit));
    if (m_phase == gc_phase::MARK) {
        byte* obj_start = gc_tagging::get_obj_start(ptr);
        if (obj_start) {
            gc_cell cell = gc_index_object(obj_start);
            if (!cell.get_mark()) {
                m_remset.add(obj_start);
            }
        }
    }
}

void gc_incremental::interior_wbarrier(gc_handle& handle, ptrdiff_t offset)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(handle);
    if (gc_tagging::is_derived(ptr)) {
        gc_tagging::reset_derived_ptr(ptr, offset);
    } else {
        dptr_descriptor* descr = m_dptr_storage.make_derived(ptr, offset);
        byte* dptr = gc_tagging::make_derived_ptr(descr, gc_tagging::is_root(ptr));
        gc_handle_access::set<std::memory_order_relaxed>(handle, dptr);
    }
}

gc_run_stats gc_incremental::gc(const gc_options& options)
{
    if (options.kind == gc_kind::CONCURRENT_MARK && m_phase == gc_phase::IDLE) {
        return start_marking();
    } else if (options.kind == gc_kind::COLLECT) {
        gc_run_stats stats = sweep(options);
        allocators::gc_core_allocator::shrink();
        return stats;
    }
    return gc_run_stats();
}

gc_run_stats gc_incremental::start_marking()
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE);

    world_snapshot snapshot = thread_manager::instance().stop_the_world();
    m_marker.trace_roots(snapshot.get_root_tracer());
    m_phase = gc_phase::MARK;
    m_marker.concurrent_mark(std::max((size_t) 1, m_threads_available - 1));

    gc_run_stats stats;
    stats.pause_stat.type     = gc_pause_type::TRACE_ROOTS;
    stats.pause_stat.duration = snapshot.time_since_stop_the_world();
    return stats;
}

gc_run_stats gc_incremental::sweep(const gc_options& options)
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE || m_phase == gc_phase::MARK);

    gc_pause_type type;
    world_snapshot snapshot = thread_manager::instance().stop_the_world();
    if (m_phase == gc_phase::IDLE) {
        type = gc_pause_type::MARK_COLLECT;
        m_marker.trace_roots(snapshot.get_root_tracer());
        m_marker.trace_pins(snapshot.get_pin_tracer());
        m_marker.trace_remset();
        m_phase = gc_phase::MARK;
        m_marker.concurrent_mark(m_threads_available - 1);
        m_marker.mark();
    } else if (m_phase == gc_phase::MARK) {
        type = gc_pause_type::COLLECT;
        m_marker.trace_pins(snapshot.get_pin_tracer());
        m_marker.trace_remset();
        m_marker.mark();
    }
    m_phase = gc_phase::COLLECT;

    m_dptr_storage.destroy_unmarked();

    gc_run_stats stats;

    stats.heap_stat           = m_heap.collect(snapshot, options.gen, m_threads_available);
    stats.pause_stat.type     = type;
    stats.pause_stat.duration = snapshot.time_since_stop_the_world();

    m_phase = gc_phase::IDLE;

    return stats;
}

gc_info gc_incremental::info() const
{
    static gc_info inf = {
            .incremental_flag                = true,
            .support_concurrent_marking      = true,
            .support_concurrent_collecting   = false
    };

    return inf;
}

}}}

