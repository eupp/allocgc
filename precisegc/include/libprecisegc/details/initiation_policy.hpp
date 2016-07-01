#ifndef DIPLOMA_INITATION_POLICY_HPP
#define DIPLOMA_INITATION_POLICY_HPP

#include <limits>

#include <libprecisegc/details/gc_strategy.hpp>

namespace precisegc { namespace details {

class initiation_policy
{
public:
    virtual ~initiation_policy() {}

    virtual gc_phase check(initiation_point_type ipoint, const gc_state& state) const = 0;
    virtual void update(const gc_state& state) = 0;
};

class empty_policy : public initiation_policy
{
public:
    gc_phase check(initiation_point_type ipoint, const gc_state& state) const override
    {
        return gc_phase::IDLE;
    }

    void update(const gc_state& state) override
    {
        return;
    }
};

class space_based_policy : public initiation_policy
{
public:
    space_based_policy(size_t start_heap_size,
                       double marking_threshold,
                       double sweeping_threshold,
                       double increase_factor,
                       size_t max_heap_size = std::numeric_limits<size_t>::max());

    gc_phase check(initiation_point_type ipoint, const gc_state& state) const override;
    void update(const gc_state& state) override;

    size_t heap_size() const;
    size_t max_heap_size() const;
    double marking_threshold() const;
    double sweeping_threshold() const;
    double increase_factor() const;
private:
    size_t m_heap_size;
    const size_t m_max_heap_size;
    const double m_marking_threshold;
    const double m_sweeping_threshold;
    const double m_increase_factor;
};

}}

#endif //DIPLOMA_INITATION_POLICY_HPP
