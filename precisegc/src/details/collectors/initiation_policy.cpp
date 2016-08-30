#include <libprecisegc/details/initiation_policy.hpp>

#include <libprecisegc/details/gc_hooks.hpp>

namespace precisegc { namespace details {

space_based_policy::space_based_policy(size_t start_heap_size,
                                       double marking_threshold,
                                       double sweeping_threshold,
                                       double increase_factor,
                                       size_t max_heap_size)
    : m_heap_size(start_heap_size)
    , m_max_heap_size(max_heap_size)
    , m_marking_threshold(marking_threshold)
    , m_sweeping_threshold(sweeping_threshold)
    , m_increase_factor(increase_factor)
{
    if (m_marking_threshold < 0.0 || m_marking_threshold > 1.0) {
        throw std::invalid_argument("marking_threshold argument for space_based_policy should lie in range [0.0, 1.0]");
    }
    if (m_sweeping_threshold < 0.0 || m_sweeping_threshold > 1.0) {
        throw std::invalid_argument("sweeping_threshold argument for space_based_policy should lie in range [0.0, 1.0]");
    }
    if (m_increase_factor < 1.0) {
        throw std::invalid_argument("increase_factor argument for space_based_policy should be greater or equal to 1.0");
    }
}

void space_based_policy::initiation_point(gc_launcher* launcher,
                                          initiation_point_type ipt, const initiation_point_data& ipd)
{
    if (ipt == initiation_point_type::HEAP_EXPANSION) {
        size_t size = launcher->state().heap_size + ipd.alloc_size();
        if (size > m_sweeping_threshold * m_heap_size) {
            launcher->gc(gc_phase::COLLECT);
            size_t new_size = launcher->state().heap_size + ipd.alloc_size();
            if (new_size > m_max_heap_size) {
                throw gc_bad_alloc();
            }
            if (new_size > m_sweeping_threshold * m_heap_size) {
                size_t increased_size = m_increase_factor * m_heap_size;
                m_heap_size = std::min(increased_size, m_max_heap_size);
            }
            return;
        }
        if (size > m_marking_threshold * m_heap_size) {
            return launcher->gc(gc_phase::MARK);
        }
    }
}

size_t space_based_policy::heap_size() const
{
    return m_heap_size;
}

size_t space_based_policy::max_heap_size() const
{
    return m_max_heap_size;
}

double space_based_policy::marking_threshold() const
{
    return m_marking_threshold;
}

double space_based_policy::sweeping_threshold() const
{
    return m_sweeping_threshold;
}

double space_based_policy::increase_factor() const
{
    return m_increase_factor;
}

}}
