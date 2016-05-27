#include "initator.hpp"

#include <atomic>
#include <stdexcept>
#include <utility>

#include "gc_heap.h"
#include "gc_garbage_collector.h"
#include "logging.h"

namespace precisegc { namespace details {

initator::initator(serial_gc_interface* gc, std::unique_ptr<initation_policy>&& policy)
    : m_gc(gc)
    , m_policy(std::move(policy))
{
    assert(m_gc);
    assert(m_policy);
}

void initator::initation_point(initation_point_type ipoint)
{
    if (m_policy->check(m_gc->stat(), ipoint)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        gc_stat stat = m_gc->stat();
        if (m_policy->check(stat, ipoint)) {
            m_gc->gc();
            gc_stat new_stat = m_gc->stat();
            m_policy->update(new_stat, ipoint);
        }
    }
}

initation_policy* initator::get_policy() const
{
    return m_policy.get();
}

incremental_initator::incremental_initator(incremental_gc_interface* gc,
                                           std::unique_ptr<incremental_initation_policy>&& policy)
    : m_gc(gc)
    , m_policy(std::move(policy))
{
    assert(m_gc);
    assert(m_policy);
}

void incremental_initator::initation_point(initation_point_type ipoint)
{
    gc_phase phase = m_policy->check(m_gc->stat(), ipoint);
    if (phase != gc_phase::IDLING) {
        std::lock_guard<std::mutex> lock(m_mutex);
        gc_stat stat = m_gc->stat();
        phase = m_policy->check(stat, ipoint);
        gc_phase curr_phase = m_gc->phase();
        if (phase == gc_phase::MARKING && curr_phase == gc_phase::IDLING) {
            incremental_gc_ops ops;
            ops.phase = phase;
            ops.concurrent_flag = stat.support_concurrent_mark;
            ops.threads_num = 1;
            m_gc->incremental_gc(ops);
        } else if (phase == gc_phase::SWEEPING
                   && (curr_phase == gc_phase::IDLING || curr_phase == gc_phase::MARKING)) {
            incremental_gc_ops ops;
            ops.phase = phase;
            ops.concurrent_flag = stat.support_concurrent_sweep;
            ops.threads_num = 1;
            m_gc->incremental_gc(ops);
        }
    }
}

incremental_initation_policy* incremental_initator::get_policy() const
{
    return m_policy.get();
}


static size_t mem_lower_bound = 0;
static size_t mem_upper_bound = 0;
static std::atomic<size_t> alloc_ticks(0);

static double b_to_mb(size_t size)
{
    return size / (1024.0 * 1024.0);
}

void init_initator(size_t lower_bound, size_t upper_bound)
{
    mem_lower_bound = lower_bound;
    mem_upper_bound = upper_bound;
}

void initate_gc()
{
//    return;
    ++alloc_ticks;
    if (alloc_ticks < 3000) {
        return;
    }
    gc_heap& heap = gc_heap::instance();
    gc_garbage_collector& garbage_collector = gc_garbage_collector::instance();
    size_t mem = heap.size();
    if (mem > mem_lower_bound ) {
        logging::debug() << "Heap size exceeded " << b_to_mb(mem_lower_bound);
        garbage_collector.start_marking();
    }
    if (mem > mem_upper_bound) {
        logging::debug() << "Heap size exceeded " << b_to_mb(mem_upper_bound);
        alloc_ticks.store(0);
        garbage_collector.start_compacting();
    }
}

}}