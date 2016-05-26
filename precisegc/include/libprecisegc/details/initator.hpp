#ifndef DIPLOMA_GC_INITATOR_H
#define DIPLOMA_GC_INITATOR_H

#include <atomic>
#include <limits>
#include <list>
#include <memory>
#include <mutex>

#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/initation_policy.hpp>
#include <libprecisegc/details/utils/utility.hpp>

#include "gc_heap.h"
#include "gc_garbage_collector.h"

namespace precisegc { namespace details {

class initator : private utils::noncopyable, private utils::nonmovable
{
public:
    initator(serial_gc_interface* gc, std::unique_ptr<initation_policy>&& policy);

    void initation_point(initation_point_type ipoint);
private:
    serial_gc_interface* m_gc;
    std::unique_ptr<initation_policy> m_policy;
    std::mutex m_mutex;
};

class incremental_initator : private utils::noncopyable, private utils::nonmovable
{
public:
    incremental_initator(incremental_gc_interface* gc, std::unique_ptr<incremental_initation_policy>&& policy);

    void initation_point(initation_point_type ipoint);
private:
    incremental_gc_interface* m_gc;
    std::unique_ptr<incremental_initation_policy> m_policy;
    std::mutex m_mutex;
};

class space_based_incremental_initator : public internals::space_based_initator_base
{
public:
    space_based_incremental_initator(incremental_gc_interface* gc,
                                     size_t start_heap_size,
                                     double marking_threshold,
                                     double compacting_threshold,
                                     double increase_factor,
                                     size_t max_heap_size = std::numeric_limits<size_t>::max());

    void initation_point();
private:
    void initate_full_gc();
    size_t get_managed_heap_size() const;

    incremental_gc_interface* m_gc;
    const double m_marking_threshold;
};

void init_initator(size_t lower_bound, size_t upper_bound);
void initate_gc();


}}

#endif //DIPLOMA_GC_INITATOR_H
