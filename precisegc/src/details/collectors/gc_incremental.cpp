#include <libprecisegc/details/collectors/gc_incremental.hpp>

#include <cassert>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/threads/gc_thread_manager.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/allocators/memory_index.hpp>

namespace precisegc { namespace details { namespace collectors {

gc_incremental::gc_incremental(const gc_factory::options& opt, const thread_descriptor& main_thrd_descr)
    : gc_core(opt, main_thrd_descr, &m_remset)
    , m_phase(gc_phase::IDLE)
    , m_threads_available(opt.threads_available)
{}

gc_alloc::response gc_incremental::allocate(const gc_alloc::request& rqst)
{
    try {
        return gc_core::allocate(rqst);
    } catch (gc_bad_alloc& exc) {
        return gc_core::allocate(rqst);
    }
}

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
    byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(src);
    gc_handle_access::set<std::memory_order_release>(dst, ptr);
    if (m_phase == gc_phase::MARK && ptr) {
        gc_cell cell = allocators::memory_index::get_gc_cell(ptr);
        if (!cell.get_mark()) {
            m_remset.add(ptr);
        }
    }
}

gc_run_stat gc_incremental::gc(const gc_options& options)
{
    if (options.kind == gc_kind::CONCURRENT_MARK && m_phase == gc_phase::IDLE) {
        return start_marking_phase();
    } else if (options.kind == gc_kind::COLLECT) {
        gc_run_stat stats = sweep();
        allocators::gc_core_allocator::shrink();
        return stats;
    }
    return gc_run_stat();
}

gc_run_stat gc_incremental::start_marking_phase()
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE);

    auto snapshot = stop_the_world();
    trace_uninit(snapshot);
    trace_roots(snapshot);
    m_phase = gc_phase::MARK;
    start_concurrent_marking(std::max((size_t) 1, m_threads_available - 1));

    gc_run_stat stats;
    stats.pause_stat.type     = gc_pause_type::TRACE_ROOTS;
    stats.pause_stat.duration = snapshot.time_since_stop_the_world();
    return stats;
}

gc_run_stat gc_incremental::sweep()
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE || m_phase == gc_phase::MARK);

    gc_pause_type type;
    world_snapshot snapshot = stop_the_world();
    if (m_phase == gc_phase::IDLE) {
        type = gc_pause_type::MARK_COLLECT;
        trace_uninit(snapshot);
        trace_roots(snapshot);
        trace_pins(snapshot);
        trace_remset();
        m_phase = gc_phase::MARK;
        start_concurrent_marking(m_threads_available - 1);
        start_marking();
    } else if (m_phase == gc_phase::MARK) {
        type = gc_pause_type::COLLECT;
        trace_uninit(snapshot);
        trace_pins(snapshot);
        trace_remset();
        start_marking();
    }
    m_phase = gc_phase::COLLECT;

    gc_run_stat stats;

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

