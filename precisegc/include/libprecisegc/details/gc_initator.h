#ifndef DIPLOMA_GC_INITATOR_H
#define DIPLOMA_GC_INITATOR_H

#include <mutex>

#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/utils/utility.hpp>

#include "gc_heap.h"
#include "gc_garbage_collector.h"

namespace precisegc { namespace details {

namespace internals {

class gc_negotiator : private utils::noncopyable, private utils::nonmovable
{
public:
    gc_negotiator(gc_interface* gc);

    void request_marking();
    void request_compacting();
    void request_idle();
private:
    gc_interface* m_gc;
    std::mutex m_mutex;
};

}

void init_initator(size_t lower_bound, size_t upper_bound);
void initate_gc();


}}

#endif //DIPLOMA_GC_INITATOR_H
