#include <libprecisegc/details/collectors/incremental_gc.hpp>

#include <cassert>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/threads/thread_manager.hpp>
#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/threads/world_snapshot.hpp>

namespace precisegc { namespace details { namespace collectors {

namespace internals {

incremental_gc_base::incremental_gc_base(gc_compacting compacting, size_t threads_available)
    : m_heap(compacting)
    , m_marker(&m_packet_manager)
    , m_phase(gc_phase::IDLE)
    , m_threads_available(threads_available)
{}

managed_ptr incremental_gc_base::allocate(size_t size)
{
    gc_unsafe_scope unsafe_scope;
    managed_ptr mp = m_heap.allocate(size);
    if (m_phase == gc_phase::MARK) {
        mp.set_mark(true);
    }
    return mp;
}

byte* incremental_gc_base::rbarrier(const gc_handle& handle)
{
    return gc_handle_access::load(handle, std::memory_order_acquire);
}

void incremental_gc_base::wbarrier(gc_handle& dst, const gc_handle& src)
{
    gc_unsafe_scope unsafe_scope;
    byte* p = gc_handle_access::load(src, std::memory_order_acquire);
    gc_handle_access::store(dst, p, std::memory_order_release);
    if (m_phase == gc_phase::MARK) {
        managed_ptr mp(p);
        if (mp && !mp.get_mark()) {
            std::unique_ptr<mark_packet>& packet = threads::managed_thread::this_thread().get_mark_packet();
            if (!packet) {
                packet = m_packet_manager.pop_output_packet();
            } else if (packet->is_full()) {
                auto new_packet = m_packet_manager.pop_output_packet();
                m_packet_manager.push_packet(std::move(packet));
                packet = std::move(new_packet);
            }
            packet->push(mp);
        }
    }
}

void incremental_gc_base::interior_wbarrier(gc_handle& handle, byte* ptr)
{
    gc_handle_access::store(handle, ptr, std::memory_order_release);
}

void incremental_gc_base::interior_shift(gc_handle& handle, ptrdiff_t shift)
{
    gc_handle_access::fetch_advance(handle, shift, std::memory_order_acq_rel);
}

void incremental_gc_base::gc(gc_phase phase)
{
    if (phase == gc_phase::MARK && m_phase == gc_phase::IDLE) {
        start_marking();
    } else if (phase == gc_phase::SWEEP) {
        sweep();
    }
}

void incremental_gc_base::flush_threads_packets(const threads::world_snapshot& snapshot)
{
    snapshot.apply_to_threads([this] (threads::managed_thread* thread ) {
        if (thread->get_mark_packet()) {
            m_packet_manager.push_packet(std::move(thread->get_mark_packet()));
            thread->get_mark_packet() = nullptr;
        }
    });
}

void incremental_gc_base::start_marking()
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE);

    world_snapshot snapshot = thread_manager::instance().stop_the_world();
    m_marker.trace_roots(snapshot.get_root_tracer());
    m_phase = gc_phase::MARK;
    m_marker.concurrent_mark(std::max((size_t) 1, m_threads_available - 1));

    gc_pause_stat pause_stat = {
            .type       = gc_pause_type::TRACE_ROOTS,
            .duration   = snapshot.time_since_stop_the_world()
    };
    gc_register_pause(pause_stat);
}

void incremental_gc_base::sweep()
{
    using namespace threads;
    assert(m_phase == gc_phase::IDLE || m_phase == gc_phase::MARK);

    gc_pause_type pause_type;
    world_snapshot snapshot = thread_manager::instance().stop_the_world();
    if (m_phase == gc_phase::IDLE) {
        pause_type = gc_pause_type::GC;
        m_marker.trace_roots(snapshot.get_root_tracer());
        m_marker.trace_pins(snapshot.get_pin_tracer());
        m_phase = gc_phase::MARK;
        m_marker.concurrent_mark(m_threads_available - 1);
        m_marker.mark();
    } else if (m_phase == gc_phase::MARK) {
        pause_type = gc_pause_type::SWEEP_HEAP;
        m_marker.trace_pins(snapshot.get_pin_tracer());
        flush_threads_packets(snapshot);
        m_marker.mark();
    }
    m_phase = gc_phase::SWEEP;

    gc_sweep_stat sweep_stat = m_heap.sweep(snapshot, m_threads_available);
    gc_pause_stat pause_stat = {
            .type       = pause_type,
            .duration   = snapshot.time_since_stop_the_world()
    };

    gc_register_sweep(sweep_stat, pause_stat);
    m_phase = gc_phase::IDLE;
}

}

incremental_gc::incremental_gc(size_t threads_available)
    : incremental_gc_base(gc_compacting::DISABLED, threads_available)
{}

bool incremental_gc::compare(const gc_handle& a, const gc_handle& b)
{
    return gc_handle_access::load(a, std::memory_order_acquire) == gc_handle_access::load(b, std::memory_order_acquire);
}

byte* incremental_gc::pin(const gc_handle& handle)
{
    return gc_handle_access::load(handle, std::memory_order_acquire);
}

void incremental_gc::unpin(byte* ptr)
{
    return;
}

gc_info incremental_gc::info() const
{
    static gc_info inf = {
            .incremental_flag           = true,
            .support_concurrent_mark    = true,
            .support_concurrent_sweep   = false
    };

    return inf;
}

incremental_compacting_gc::incremental_compacting_gc(size_t threads_available)
        : incremental_gc_base(gc_compacting::ENABLED, threads_available)
{}

bool incremental_compacting_gc::compare(const gc_handle& a, const gc_handle& b)
{
    gc_unsafe_scope unsafe_scope;
    return gc_handle_access::load(a, std::memory_order_acquire) == gc_handle_access::load(b, std::memory_order_acquire);
}

byte* incremental_compacting_gc::pin(const gc_handle& handle)
{
    gc_unsafe_scope unsafe_scope;
    byte* ptr = gc_handle_access::load(handle, std::memory_order_acquire);
    if (ptr) {
        static thread_local pin_stack_map& pin_set = threads::managed_thread::this_thread().pin_set();
        pin_set.insert(ptr);
    }
    return ptr;
}

void incremental_compacting_gc::unpin(byte* ptr)
{
    static thread_local pin_stack_map& pin_set = threads::managed_thread::this_thread().pin_set();
    pin_set.remove(ptr);
}

gc_info incremental_compacting_gc::info() const
{
    static gc_info inf = {
            .incremental_flag           = true,
            .support_concurrent_mark    = true,
            .support_concurrent_sweep   = false
    };

    return inf;
}

}}}

