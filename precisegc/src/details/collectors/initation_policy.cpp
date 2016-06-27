#include <libprecisegc/details/collectors/initation_policy.hpp>

namespace precisegc { namespace details { namespace collectors {

space_based_policy::space_based_policy(size_t start_heap_size,
                                       double threshold,
                                       double increase_factor,
                                       size_t max_heap_size)
    : m_heap_size(start_heap_size)
    , m_max_heap_size(max_heap_size)
    , m_threshold(threshold)
    , m_increase_factor(increase_factor)
{
    assert(m_heap_size.is_lock_free());
    if (m_threshold < 0.0 || m_threshold > 1.0) {
        throw std::invalid_argument("threshold argument for space_based_policy should lie in range [0.0, 1.0]");
    }
    if (m_increase_factor < 1.0) {
        throw std::invalid_argument("increase_factor argument for space_based_policy should be greater or equal to 1.0");
    }
}

bool space_based_policy::check(const gc_stat& stat, initation_point_type ipoint) const
{
    if (ipoint == initation_point_type::HEAP_GROWTH) {
        return stat.heap_size > m_threshold * heap_size();
    }
    return false;
}

void space_based_policy::update(const gc_stat& stat, initation_point_type ipoint)
{
    if (ipoint == initation_point_type::HEAP_GROWTH) {
        size_t curr_max_heap_size = heap_size();
        if (stat.heap_size > m_threshold * curr_max_heap_size) {
            if (stat.heap_size > m_max_heap_size) {
                throw gc_bad_alloc("heap size exceeded max size");
            }
            size_t increased_size = m_increase_factor * curr_max_heap_size;
            size_t new_max_size = std::min(increased_size, m_max_heap_size);
            m_heap_size.store(new_max_size, std::memory_order_release);
        }
    }
}

size_t space_based_policy::heap_size() const
{
    return m_heap_size.load(std::memory_order_acquire);
}

size_t space_based_policy::max_heap_size() const
{
    return m_max_heap_size;
}

double space_based_policy::threshold() const
{
    return m_threshold;
}

double space_based_policy::increase_factor() const
{
    return m_increase_factor;
}

incremental_space_based_policy::incremental_space_based_policy(size_t start_heap_size,
                                                               double marking_threshold,
                                                               double sweeping_threshold,
                                                               double increase_factor,
                                                               size_t max_heap_size)
    : m_sweeping_policy(start_heap_size, sweeping_threshold, increase_factor, max_heap_size)
    , m_marking_threshold(marking_threshold)
{
    if (m_marking_threshold < 0.0 || m_marking_threshold > 1.0) {
        throw std::invalid_argument("marking_threshold argument for incremental_space_based_policy should lie in range [0.0, 1.0]");
    }
}

gc_phase incremental_space_based_policy::check(const gc_stat& stat, initation_point_type ipoint) const
{
    if (m_sweeping_policy.check(stat, ipoint)) {
        return gc_phase::SWEEPING;
    } else if (ipoint == initation_point_type::HEAP_GROWTH && stat.heap_size > m_marking_threshold * heap_size()) {
        return gc_phase::MARKING;
    }
    return gc_phase::IDLING;
}

void incremental_space_based_policy::update(const gc_stat& stat, initation_point_type ipoint)
{
    m_sweeping_policy.update(stat, ipoint);
}

size_t incremental_space_based_policy::heap_size() const
{
    return m_sweeping_policy.heap_size();
}

size_t incremental_space_based_policy::max_heap_size() const
{
    return m_sweeping_policy.max_heap_size();
}

double incremental_space_based_policy::marking_threshold() const
{
    return m_marking_threshold;
}

double incremental_space_based_policy::sweeping_threshold() const
{
    return m_sweeping_policy.threshold();
}

double incremental_space_based_policy::increase_factor() const
{
    return m_sweeping_policy.increase_factor();
}

}}}
