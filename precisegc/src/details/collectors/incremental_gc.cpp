#include <libprecisegc/details/collectors/incremental_gc.hpp>

#include <cassert>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/threads/this_managed_thread.hpp>

namespace precisegc { namespace details { namespace collectors {

namespace internals {

incremental_gc_base::incremental_gc_base(gc_compacting compacting, size_t threads_available)
    : m_heap(compacting)
    , m_marker(&m_packet_manager, &m_remset)
    , m_phase(gc_phase::IDLE)
    , m_threads_available(threads_available)
{}

gc_alloc_descriptor incremental_gc_base::allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta)
{
    gc_alloc_descriptor descr = m_heap.allocate(sizeof(traceable_object_meta) + obj_size * obj_cnt);
    traceable_object_meta* meta = managed_object::get_meta(descr.get());
    new (meta) traceable_object_meta(obj_cnt, tmeta);
    return gc_alloc_descriptor(managed_object::get_object(descr.get()),
                               descr.size() - sizeof(traceable_object_meta),
                               descr.descriptor());
}

void incremental_gc_base::commit(const gc_alloc_descriptor& alloc_descr)
{
    if (m_phase == gc_phase::MARK) {
        alloc_descr.descriptor()->set_mark(alloc_descr.get(), true);
    }
}

byte* incremental_gc_base::rbarrier(const gc_word& handle)
{
    return dptr_storage::get(gc_handle_access::get<std::memory_order_relaxed>(handle));
}

void incremental_gc_base::wbarrier(gc_word& dst, const gc_word& src)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(src);
    gc_handle_access::set<std::memory_order_release>(dst, ptr);
    if (m_phase == gc_phase::MARK) {
        indexed_managed_object idx_obj = indexed_managed_object::index(dptr_storage::get_origin(ptr));
        if (idx_obj && !idx_obj.get_mark()) {
            m_remset.add(idx_obj.object());
        }
    }
}

void incremental_gc_base::interior_wbarrier(gc_word& handle, ptrdiff_t offset)
{
    byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(handle);
    if (dptr_storage::is_derived(ptr)) {
        dptr_storage::reset_derived_ptr(ptr, offset);
    } else {
        byte* derived = m_dptr_storage.make_derived(ptr, offset);
        gc_handle_access::set<std::memory_order_relaxed>(handle, derived);
    }
}

gc_run_stats incremental_gc_base::gc(const gc_options& options)
{
    if (options.phase == gc_phase::MARK && m_phase == gc_phase::IDLE) {
        return start_marking();
    } else if (options.phase == gc_phase::COLLECT) {
        return sweep();
    }
    gc_run_stats stats = {
            .type           = gc_type::SKIP_GC,
            .mem_swept      = 0,
            .mem_copied     = 0,
            .pause_duration = gc_clock::duration(0)
    };
    return stats;
}

gc_run_stats incremental_gc_base::start_marking()
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE);

    world_snapshot snapshot = thread_manager::instance().stop_the_world();
    m_marker.trace_roots(snapshot.get_root_tracer());
    m_phase = gc_phase::MARK;
    m_marker.concurrent_mark(std::max((size_t) 1, m_threads_available - 1));

    gc_run_stats stats = {
            .type           = gc_type::TRACE_ROOTS,
            .mem_swept      = 0,
            .mem_copied     = 0,
            .pause_duration = snapshot.time_since_stop_the_world()
    };

    return stats;
}

gc_run_stats incremental_gc_base::sweep()
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE || m_phase == gc_phase::MARK);

    gc_type type;
    world_snapshot snapshot = thread_manager::instance().stop_the_world();
    if (m_phase == gc_phase::IDLE) {
        type = gc_type::FULL_GC;
        m_marker.trace_roots(snapshot.get_root_tracer());
        m_marker.trace_pins(snapshot.get_pin_tracer());
        m_marker.trace_remset();
        m_phase = gc_phase::MARK;
        m_marker.concurrent_mark(m_threads_available - 1);
        m_marker.mark();
    } else if (m_phase == gc_phase::MARK) {
        type = gc_type::COLLECT_GARBAGE;
        m_marker.trace_pins(snapshot.get_pin_tracer());
        m_marker.trace_remset();
        m_marker.mark();
    }
    m_phase = gc_phase::COLLECT;
    m_dptr_storage.destroy_unmarked();
    auto collect_stats = m_heap.collect(snapshot, m_threads_available);
    m_phase = gc_phase::IDLE;

    gc_run_stats stats = gc_run_stats {
            .type           = type,
            .mem_swept      = collect_stats.mem_swept,
            .mem_copied     = collect_stats.mem_copied,
            .pause_duration = snapshot.time_since_stop_the_world()
    };

    return stats;
}

}

incremental_gc::incremental_gc(size_t threads_available)
    : incremental_gc_base(gc_compacting::DISABLED, threads_available)
{}

gc_info incremental_gc::info() const
{
    static gc_info inf = {
            .incremental_flag           = true,
            .support_concurrent_marking    = true,
            .support_concurrent_collecting   = false
    };

    return inf;
}

incremental_compacting_gc::incremental_compacting_gc(size_t threads_available)
        : incremental_gc_base(gc_compacting::ENABLED, threads_available)
{}

void incremental_compacting_gc::interior_wbarrier(gc_word& handle, ptrdiff_t offset)
{
    gc_unsafe_scope unsafe_scope;
    internals::incremental_gc_base::interior_wbarrier(handle, offset);
}

gc_info incremental_compacting_gc::info() const
{
    static gc_info inf = {
            .incremental_flag                = true,
            .support_concurrent_marking      = true,
            .support_concurrent_collecting   = false
    };

    return inf;
}

}}}

