#include <libprecisegc/details/incremental_garbage_collector.hpp>

#include <cassert>

#include <libprecisegc/details/gc_unsafe_scope.h>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/world_state.hpp>

namespace precisegc { namespace details {

incremental_garbage_collector::incremental_garbage_collector(gc_compacting compacting,
                                                             std::unique_ptr<incremental_initation_policy> init_policy)
    : m_initator(this, std::move(init_policy))
    , m_heap(compacting)
    , m_phase(gc_phase::IDLING)
{
    assert(m_phase.is_lock_free());
}

managed_ptr incremental_garbage_collector::allocate(size_t size)
{
    return m_heap.allocate(size);
}

byte* incremental_garbage_collector::rbarrier(const atomic_byte_ptr& p)
{
    return p.load(std::memory_order_acquire);
}

void incremental_garbage_collector::wbarrier(atomic_byte_ptr& dst, const atomic_byte_ptr& src)
{
    gc_unsafe_scope unsafe_scope;
    dst.store(src.load(std::memory_order_acquire), std::memory_order_release);
}

void incremental_garbage_collector::initation_point(initation_point_type ipoint)
{
    m_initator.initation_point(ipoint);
}

gc_stat incremental_garbage_collector::stat() const
{
    gc_stat stat;
    stat.heap_size                  = m_heap.size();
    stat.incremental                = true;
    stat.support_concurrent_mark    = true;
    stat.support_concurrent_sweep   = false;
    return stat;
}

gc_phase incremental_garbage_collector::phase() const
{
    return m_phase.load(std::memory_order_acquire);
}

void incremental_garbage_collector::set_phase(gc_phase phase)
{
    m_phase.store(phase, std::memory_order_release);
}

void incremental_garbage_collector::gc()
{
    sweep();
}

void incremental_garbage_collector::incremental_gc(const incremental_gc_ops& ops)
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

void incremental_garbage_collector::start_marking()
{
    using namespace threads;
    assert(phase() == gc_phase::IDLING);
    world_state wstate = thread_manager::instance().stop_the_world();
    m_marker.trace_roots(wstate);
    set_phase(gc_phase::MARKING);
    m_marker.start_marking();
}

void incremental_garbage_collector::sweep()
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

