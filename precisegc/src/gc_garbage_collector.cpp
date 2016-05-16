#include "gc_garbage_collector.h"

#include <cstdint>
#include <unistd.h>

#include "gc_ptr.h"
#include "gc_mark.h"
#include "barrier_buffer.h"
#include "managed_ptr.h"
#include "gc_heap.h"
#include "logging.h"

using namespace _GC_;

namespace precisegc { namespace details {

inline long long nanotime( void ) {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000ll + ts.tv_nsec;
}

inline timespec add(const timespec& a, const timespec& b)
{
    static const long BILLION = 1000000000;
    timespec res;
    res.tv_sec = a.tv_sec + b.tv_sec;
    res.tv_nsec = a.tv_nsec + b.tv_nsec;
    if (res.tv_nsec >= BILLION) {
        res.tv_nsec -= BILLION;
        res.tv_sec++;
    }
    return res;
}

inline timespec now()
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
}

static const long WAIT_TIMEOUT_MS = 100;

inline timespec abs_timeout()
{
    static timespec wait_timeout = {.tv_sec = 0, .tv_nsec = WAIT_TIMEOUT_MS * 1000};
    return add(now(), wait_timeout);
}

threads::thread_manager& gc_garbage_collector::thread_manager = threads::thread_manager::instance();

gc_garbage_collector& gc_garbage_collector::instance()
{
    static gc_garbage_collector collector;
    return collector;
}

gc_garbage_collector::gc_garbage_collector()
    : m_phase(phase::IDLE)
    , m_gc_cycles_cnt(0)
{
    assert(m_phase.is_lock_free());
}

gc_garbage_collector::~gc_garbage_collector()
{}

size_t gc_garbage_collector::get_gc_cycles_count() const
{
    return m_gc_cycles_cnt;
}

void gc_garbage_collector::start_marking()
{
//    gc_unsafe_scope unsafe_scope;
    phase phs = m_phase.load();
    if (phs == phase::IDLE) {
        run_marking();
    }
}

void gc_garbage_collector::start_compacting()
{
//    gc_unsafe_scope unsafe_scope;
    phase phs = m_phase.load();
    if (phs == phase::IDLE) {
        run_gc();
    } else if (phs == phase::MARKING) {
        run_compacting();
    }
}

void gc_garbage_collector::force_move_to_idle()
{
    thread_manager.stop_the_world();
    m_phase.store(phase::IDLE);
    thread_manager.start_the_world();
}

void gc_garbage_collector::run_marking()
{
    long long start = nanotime();

    thread_manager.stop_the_world();
    m_phase.store(phase::MARKING);
    m_marker.trace_roots();
    thread_manager.start_the_world();

    printf("trace roots stw time = %lld microsec \n", (nanotime() - start) / 1000);

    m_marker.start_marking();
}

void gc_garbage_collector::run_compacting()
{
    long long start = nanotime();

    m_marker.pause_marking();
    thread_manager.stop_the_world();
    m_marker.trace_pins();
    m_marker.mark();
    m_phase.store(phase::COMPACTING);
    gc_heap::instance().compact();
    m_phase.store(phase::IDLE);
    ++m_gc_cycles_cnt;
    thread_manager.start_the_world();

    printf("heap size = %lld bytes \n", gc_heap::instance().size());
    printf("compact stw time = %lld microsec \n", (nanotime() - start) / 1000);
}

void gc_garbage_collector::run_gc()
{
    long long start = nanotime();

    m_marker.pause_marking();
    thread_manager.stop_the_world();
    m_phase.store(phase::MARKING);
    m_marker.trace_roots();
    m_marker.trace_pins();
    m_marker.mark();
    m_phase.store(phase::COMPACTING);
    gc_heap::instance().compact();
    m_phase.store(phase::IDLE);
    ++m_gc_cycles_cnt;
    thread_manager.start_the_world();

    printf("heap size = %lld bytes \n", gc_heap::instance().size());
    printf("compact stw time = %lld microsec \n", (nanotime() - start) / 1000);
}

void gc_garbage_collector::write_barrier(gc_untyped_ptr& dst_ptr, const gc_untyped_ptr& src_ptr)
{
    gc_unsafe_scope unsafe_scope;
    phase phs = m_phase.load(std::memory_order_seq_cst);
    dst_ptr.set(src_ptr.get());
    if (phs == phase::MARKING) {
        bool res = shade(&src_ptr);
        while (!res && phs == phase::MARKING) {
            m_marker.trace_barrier_buffers();
            std::this_thread::yield();
            res = shade(&src_ptr);
            phs = m_phase.load(std::memory_order_seq_cst);
        }
    }
}

void gc_garbage_collector::new_cell(managed_cell_ptr& cell_ptr)
{
    // do not call unsafe scope here
    // because new_cell is always called in the context of gc_new which is already marked as unsafe_scope
    gc_unsafe_scope unsafe_scope;
    phase phs = m_phase.load(std::memory_order_seq_cst);
    if (phs == phase::MARKING) {
        // allocate black objects
        cell_ptr.set_mark(true);
    } else {
        cell_ptr.set_mark(false);
    }
}

const char* gc_garbage_collector::phase_str(gc_garbage_collector::phase ph)
{
    switch (ph) {
        case phase::IDLE:
            return "Idle";
        case phase::MARKING:
            return "Marking";
        case phase::COMPACTING:
            return "Compacting";
    }
}

}}