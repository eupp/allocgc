#include <libprecisegc/details/incremental_gc.hpp>

#include <cassert>

#include <libprecisegc/details/gc_unsafe_scope.h>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/world_state.hpp>

namespace precisegc { namespace details {

incremental_gc::incremental_gc(gc_compacting compacting,
                                                             std::unique_ptr<incremental_initation_policy> init_policy)
    : m_initator(this, std::move(init_policy))
    , m_heap(compacting)
    , m_phase(gc_phase::IDLING)
{
    assert(m_phase.is_lock_free());
}

managed_ptr incremental_gc::allocate(size_t size)
{
    gc_unsafe_scope unsafe_scope;
    managed_ptr mp = m_heap.allocate(size);
    if (phase() == gc_phase::MARKING) {
        mp.set_mark(true);
    }
    return mp;
}

byte* incremental_gc::rbarrier(const atomic_byte_ptr& p)
{
    return p.load(std::memory_order_acquire);
}

void incremental_gc::wbarrier(atomic_byte_ptr& dst, const atomic_byte_ptr& src)
{
    gc_unsafe_scope unsafe_scope;
    byte* p = src.load(std::memory_order_acquire);
    dst.store(p, std::memory_order_release);
    if (phase() == gc_phase::MARKING) {
        bool res = shade(p);
        while (!res) {
            m_marker.trace_barrier_buffers();
            std::this_thread::yield();
            res = shade(p);
        }
    }
}

void incremental_gc::initation_point(initation_point_type ipoint)
{
    m_initator.initation_point(ipoint);
}

gc_info incremental_gc::info() const
{
    static gc_info inf = {
        .incremental                = true,
        .support_concurrent_mark    = true,
        .support_concurrent_sweep   = false
    };

    return inf;
}

gc_phase incremental_gc::phase() const
{
    return m_phase.load(std::memory_order_acquire);
}

void incremental_gc::set_phase(gc_phase phase)
{
    m_phase.store(phase, std::memory_order_release);
}

void incremental_gc::gc()
{
    sweep();
}

void incremental_gc::gc_increment(const incremental_gc_ops& ops)
{
    if (ops.phase == gc_phase::IDLING) {
        assert(phase() == gc_phase::MARKING);
        set_phase(gc_phase::IDLING);
    } else if (ops.phase == gc_phase::MARKING) {
        assert(ops.concurrent_flag);
        start_marking();
    } else if (ops.phase == gc_phase::SWEEPING) {
        assert(!ops.concurrent_flag);
        sweep();
    }
}

void incremental_gc::start_marking()
{
    using namespace threads;
    assert(phase() == gc_phase::IDLING);
    world_state wstate = thread_manager::instance().stop_the_world();
    m_marker.trace_roots(wstate);
    set_phase(gc_phase::MARKING);
    m_marker.start_marking();
}

void incremental_gc::sweep()
{
    using namespace threads;
    assert(phase() == gc_phase::IDLING || phase() == gc_phase::MARKING);
    if (phase() == gc_phase::IDLING) {
        world_state wstate = thread_manager::instance().stop_the_world();
        m_marker.trace_roots(wstate);
        m_marker.trace_pins(wstate);
        set_phase(gc_phase::MARKING);
        m_marker.mark();
    } else if (phase() == gc_phase::MARKING) {
        m_marker.pause_marking();
        world_state wstate = thread_manager::instance().stop_the_world();
        m_marker.trace_pins(wstate);
        m_marker.mark();
    }
    set_phase(gc_phase::SWEEPING);
    m_heap.sweep();
    set_phase(gc_phase::IDLING);
}

}}

