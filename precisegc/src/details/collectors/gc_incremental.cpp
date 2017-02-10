#include <libprecisegc/details/collectors/gc_incremental.hpp>

#include <cassert>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/threads/gc_thread_manager.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/collectors/memory_index.hpp>
#include <libprecisegc/details/collectors/gc_tagging.hpp>

namespace precisegc { namespace details { namespace collectors {

gc_incremental::gc_incremental(size_t threads_available, const thread_descriptor& main_thrd_descr)
    : gc_core(main_thrd_descr)
    , m_marker(&m_packet_manager, &m_remset)
    , m_phase(gc_phase::IDLE)
    , m_threads_available(threads_available)
{}

void gc_incremental::commit(const gc_alloc::response& rsp)
{
    gc_core::commit(rsp);
    if (m_phase == gc_phase::MARK) {
        gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rsp.buffer());
        stack_entry->descriptor->set_mark(rsp.cell_start(), true);
    }
}

void gc_incremental::commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta)
{
    gc_core::commit(rsp, type_meta);
    if (m_phase == gc_phase::MARK) {
        gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rsp.buffer());
        stack_entry->descriptor->set_mark(rsp.cell_start(), true);
    }
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

gc_run_stats gc_incremental::gc(const gc_options& options)
{
    if (options.kind == gc_kind::CONCURRENT_MARK && m_phase == gc_phase::IDLE) {
        return start_marking();
    } else if (options.kind == gc_kind::COLLECT) {
        gc_run_stats stats = sweep();
        allocators::gc_core_allocator::shrink();
        return stats;
    }
    return gc_run_stats();
}

gc_run_stats gc_incremental::start_marking()
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE);

    auto snapshot = stop_the_world();
    m_marker.trace_roots(snapshot, get_static_roots());
    m_phase = gc_phase::MARK;
    m_marker.concurrent_mark(std::max((size_t) 1, m_threads_available - 1));

    gc_run_stats stats;
    stats.pause_stat.type     = gc_pause_type::TRACE_ROOTS;
    stats.pause_stat.duration = snapshot.time_since_stop_the_world();
    return stats;
}

gc_run_stats gc_incremental::sweep()
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE || m_phase == gc_phase::MARK);

    gc_pause_type type;
    world_snapshot snapshot = stop_the_world();
    if (m_phase == gc_phase::IDLE) {
        type = gc_pause_type::MARK_COLLECT;
        m_marker.trace_roots(snapshot, get_static_roots());
        m_marker.trace_pins(snapshot);
        m_marker.trace_remset();
        m_phase = gc_phase::MARK;
        m_marker.concurrent_mark(m_threads_available - 1);
        m_marker.mark();
    } else if (m_phase == gc_phase::MARK) {
        type = gc_pause_type::COLLECT;
        m_marker.trace_pins(snapshot);
        m_marker.trace_remset();
        m_marker.mark();
    }
    m_phase = gc_phase::COLLECT;

    destroy_unmarked_dptrs();

    gc_run_stats stats;

    stats.heap_stat           = collect(snapshot, m_threads_available);
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

