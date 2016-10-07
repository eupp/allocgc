#ifndef DIPLOMA_INITATION_POLICY_HPP
#define DIPLOMA_INITATION_POLICY_HPP

#include <limits>

#include <libprecisegc/details/gc_interface.hpp>

namespace precisegc { namespace details {

class initiation_policy
{
public:
    virtual ~initiation_policy() {}

    virtual void initiation_point(gc_launcher* launcher,
                                  initiation_point_type ipt,
                                  const initiation_point_data& ipd) = 0;
};

class empty_policy : public initiation_policy
{
public:
    void initiation_point(gc_launcher* launcher,
                          initiation_point_type ipt,
                          const initiation_point_data& ipd) override
    {
        return;
    }
};

class space_based_policy : public initiation_policy
{
public:
    void initiation_point(gc_launcher* launcher,
                          initiation_point_type ipt,
                          const initiation_point_data& ipd) override;

    space_based_policy(size_t start_heap_size,
                       double marking_threshold,
                       double sweeping_threshold,
                       double increase_factor,
                       size_t max_heap_size = std::numeric_limits<size_t>::max());

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

class growth_based_policy : public initiation_policy
{
public:
    void initiation_point(gc_launcher* launcher,
                          initiation_point_type ipt,
                          const initiation_point_data& ipd) override;

    growth_based_policy(size_t start_heap_size,
                        double freespace_divisor,
                        size_t max_heap_size = std::numeric_limits<size_t>::max());

    size_t heap_size() const;
    size_t max_heap_size() const;
    double freespace_divisor() const;
private:
    static constexpr double MARKING_FACTOR = 2.0;

    size_t m_heap_size;
    const size_t m_max_heap_size;
    const double m_freespace_divisor;
};

}}

#endif //DIPLOMA_INITATION_POLICY_HPP
