#ifndef DIPLOMA_INITATION_POLICY_HPP
#define DIPLOMA_INITATION_POLICY_HPP

#include <limits>

#include <libprecisegc/details/gc_strategy.hpp>

namespace precisegc { namespace details {

class initiation_policy
{
public:
    virtual ~initiation_policy() {}

    virtual void initiation_point(initiation_point_type ipt,
                                  const initiation_point_data& ipd) = 0;
};

class empty_policy : public initiation_policy
{
public:
    void initiation_point(initiation_point_type ipt,
                          const initiation_point_data& ipd) override
    {
        return;
    }
};

class space_based_policy : public initiation_policy
{
public:
    space_based_policy(gc_strategy* gc,
                       size_t start_heap_size,
                       double marking_threshold,
                       double sweeping_threshold,
                       double increase_factor,
                       size_t max_heap_size = std::numeric_limits<size_t>::max());

    void initiation_point(initiation_point_type ipt,
                          const initiation_point_data& ipd) override;

    size_t heap_size() const;
    size_t max_heap_size() const;
    double marking_threshold() const;
    double sweeping_threshold() const;
    double increase_factor() const;
private:
    gc_strategy* m_gc;
    size_t m_heap_size;
    const size_t m_max_heap_size;
    const double m_marking_threshold;
    const double m_sweeping_threshold;
    const double m_increase_factor;
    const bool m_incremental_flag;
};

}}

#endif //DIPLOMA_INITATION_POLICY_HPP
