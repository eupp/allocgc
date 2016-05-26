#ifndef DIPLOMA_INITATION_POLICY_HPP
#define DIPLOMA_INITATION_POLICY_HPP

#include <limits>

#include <libprecisegc/details/gc_interface.hpp>

namespace precisegc { namespace details {

class initation_policy
{
public:
    virtual ~initation_policy() {}

    virtual bool check(const gc_stat& stat, initation_point_type ipoint) const = 0;
    virtual void update(const gc_stat& stat, initation_point_type ipoint) = 0;
};

class incremental_initation_policy
{
public:
    virtual ~incremental_initation_policy() {}

    virtual gc_phase check(const gc_stat& stat, initation_point_type ipoint) const = 0;
    virtual void update(const gc_stat& stat, initation_point_type ipoint) = 0;
};

class space_based_policy : public initation_policy
{
public:
    space_based_policy(size_t start_heap_size,
                       double threshold,
                       double increase_factor,
                       size_t max_heap_size = std::numeric_limits<size_t>::max());

    bool check(const gc_stat& stat, initation_point_type ipoint) const override;
    void update(const gc_stat& stat, initation_point_type ipoint) override ;

    size_t heap_size() const;
    size_t max_heap_size() const;
    double threshold() const;
    double increase_factor() const;
private:
    std::atomic<size_t> m_heap_size;
    const size_t m_max_heap_size;
    const double m_threshold;
    const double m_increase_factor;
};

class incremental_space_based_policy : public incremental_initation_policy
{
public:
    incremental_space_based_policy(size_t start_heap_size,
                                   double marking_threshold,
                                   double sweeping_threshold,
                                   double increase_factor,
                                   size_t max_heap_size = std::numeric_limits<size_t>::max());

    gc_phase check(const gc_stat& stat, initation_point_type ipoint) const override;
    void update(const gc_stat& stat, initation_point_type ipoint) override;

    size_t heap_size() const;
    size_t max_heap_size() const;
    double marking_threshold() const;
    double sweeping_threshold() const;
    double increase_factor() const;
private:
    space_based_policy m_sweeping_policy;
    const double m_marking_threshold;
};

}}

#endif //DIPLOMA_INITATION_POLICY_HPP
