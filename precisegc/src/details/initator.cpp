#include <libprecisegc/details/initator.hpp>

#include <atomic>
#include <stdexcept>
#include <utility>

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_heap.h>
#include <libprecisegc/details/logging.h>

namespace precisegc { namespace details {

initator::initator(serial_gc_strategy* gc, std::unique_ptr<initation_policy>&& policy)
    : m_gc(gc)
    , m_policy(std::move(policy))
{
    assert(m_gc);
    assert(m_policy);
}

void initator::initation_point(initation_point_type ipoint)
{
    if (ipoint == initation_point_type::USER_REQUEST) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_gc->gc();
        m_policy->update(gc_get_stats(), ipoint);
    }
    if (m_policy->check(gc_get_stats(), ipoint)) {
        std::lock_guard<std::mutex> lock(m_mutex);
        gc_stat stat = gc_get_stats();
        if (m_policy->check(stat, ipoint)) {
            m_gc->gc();
            m_policy->update(gc_get_stats(), ipoint);
        }
    }
}

initation_policy* initator::get_policy() const
{
    return m_policy.get();
}

incremental_initator::incremental_initator(incremental_gc_strategy* gc,
                                           std::unique_ptr<incremental_initation_policy>&& policy)
    : m_gc(gc)
    , m_policy(std::move(policy))
{
    assert(m_gc);
    assert(m_policy);
}

void incremental_initator::initation_point(initation_point_type ipoint)
{
    if (ipoint == initation_point_type::USER_REQUEST) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_gc->gc();
        m_policy->update(gc_get_stats(), ipoint);
    }
    gc_phase phase = m_policy->check(gc_get_stats(), ipoint);
    if (phase != gc_phase::IDLING) {
        std::lock_guard<std::mutex> lock(m_mutex);
        gc_stat stat = gc_get_stats();
        phase = m_policy->check(stat, ipoint);
        gc_phase curr_phase = m_gc->phase();
        if (phase == gc_phase::MARKING && curr_phase == gc_phase::IDLING) {
            incremental_gc_ops ops;
            ops.phase = phase;
            ops.concurrent_flag = gc_get_info().support_concurrent_mark;
            ops.threads_num = 1;
            m_gc->gc_increment(ops);
        } else if (phase == gc_phase::SWEEPING
                   && (curr_phase == gc_phase::IDLING || curr_phase == gc_phase::MARKING)) {
            incremental_gc_ops ops;
            ops.phase = phase;
            ops.concurrent_flag = gc_get_info().support_concurrent_sweep;
            ops.threads_num = 1;
            m_gc->gc_increment(ops);
        }
        m_policy->update(gc_get_stats(), ipoint);
    }
}

incremental_initation_policy* incremental_initator::get_policy() const
{
    return m_policy.get();
}

}}