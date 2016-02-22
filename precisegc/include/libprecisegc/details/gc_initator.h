#ifndef DIPLOMA_GC_INITATOR_H
#define DIPLOMA_GC_INITATOR_H

#include "gc_heap.h"
#include "gc_garbage_collector.h"

namespace precisegc { namespace details {

void init_initator(size_t lower_bound, size_t upper_bound);
void initate_gc();


}}

#endif //DIPLOMA_GC_INITATOR_H
