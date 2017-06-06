#include <liballocgc/details/collectors/gc_cms.hpp>

#include <cassert>

#include <liballocgc/details/gc_unsafe_scope.hpp>
#include <liballocgc/details/threads/gc_thread_manager.hpp>
#include <liballocgc/details/threads/world_snapshot.hpp>
#include <liballocgc/details/allocators/memory_index.hpp>

namespace allocgc { namespace details { namespace collectors {

gc_cms::gc_cms()
    : gc_core(&m_remset)
    , m_phase(gc_phase::IDLE)
{}

gc_alloc::response gc_cms::allocate(const gc_alloc::request& rqst)
{
    try {
        return gc_core::allocate(rqst);
    } catch (gc_bad_alloc& exc) {
        return gc_core::allocate(rqst);
    }
}

void gc_cms::commit(const gc_alloc::response& rsp)
{
    gc_core::commit(rsp);
    if (m_phase == gc_phase::MARK) {
        gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rsp.buffer());
        stack_entry->descriptor->set_mark(rsp.cell_start(), true);
    }
}

void gc_cms::commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta)
{
    gc_core::commit(rsp, type_meta);
    if (m_phase == gc_phase::MARK) {
        gc_new_stack_entry* stack_entry = reinterpret_cast<gc_new_stack_entry*>(rsp.buffer());
        stack_entry->descriptor->set_mark(rsp.cell_start(), true);
    }
}

void gc_cms::wbarrier(gc_handle& dst, const gc_handle& src)
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

gc_runstat gc_cms::gc_impl(const gc_options& options)
{
    if (options.kind == gc_kind::LAUNCH_CONCURRENT_MARK && m_phase == gc_phase::IDLE) {
        gc_runstat stats = start_marking_phase();
        return stats;
    } else if (options.kind == gc_kind::COLLECT) {
        gc_runstat stats = sweep();
        shrink();
        return stats;
    }
    return gc_runstat();
}

gc_runstat gc_cms::start_marking_phase()
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE);

    auto snapshot = stop_the_world();
    trace_uninit(snapshot);
    trace_roots(snapshot);
    m_phase = gc_phase::MARK;
    start_concurrent_marking(std::max((size_t) 1, threads_available() - 1));

    gc_runstat stats;
    stats.pause = snapshot.time_since_stop_the_world();
    return stats;
}

gc_runstat gc_cms::sweep()
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE || m_phase == gc_phase::MARK);

    world_snapshot snapshot = stop_the_world();
    if (m_phase == gc_phase::IDLE) {
        trace_uninit(snapshot);
        trace_roots(snapshot);
        trace_pins(snapshot);
        trace_remset();
        m_phase = gc_phase::MARK;
        start_concurrent_marking(threads_available() - 1);
        start_marking();
    } else if (m_phase == gc_phase::MARK) {
        trace_uninit(snapshot);
        trace_pins(snapshot);
        trace_remset();
        start_marking();
    }
    m_phase = gc_phase::COLLECT;

    gc_runstat stats;

    stats.collection = collect(snapshot, threads_available());
    stats.pause = snapshot.time_since_stop_the_world();

    m_phase = gc_phase::IDLE;

    return stats;
}

gc_info gc_cms::info() const
{
    static gc_info inf = {
            .incremental_flag                = true,
            .support_concurrent_marking      = true,
            .support_concurrent_collecting   = false
    };

    return inf;
}

}}}

