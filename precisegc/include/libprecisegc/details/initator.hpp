#ifndef DIPLOMA_GC_INITATOR_H
#define DIPLOMA_GC_INITATOR_H

#include <mutex>

#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/utils/utility.hpp>

#include "gc_heap.h"
#include "gc_garbage_collector.h"

namespace precisegc { namespace details {

class initator
{
    virtual void initation_point() = 0;
protected:
    virtual void initate_gc() = 0;
};

class space_based_initator : public initator
{
public:
    space_based_initator(gc_interface* gc)
        : m_gc(gc)
    {}

private:
    gc_interface* m_gc;
};

void init_initator(size_t lower_bound, size_t upper_bound);
void initate_gc();


}}

#endif //DIPLOMA_GC_INITATOR_H
