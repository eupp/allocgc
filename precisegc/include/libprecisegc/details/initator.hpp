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

    initation_policy* get_policy() const;
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

void init_initator(size_t lower_bound, size_t upper_bound);
void initate_gc();


}}

#endif //DIPLOMA_GC_INITATOR_H
