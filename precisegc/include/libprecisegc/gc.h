#ifndef DIPLOMA_GC_H
#define DIPLOMA_GC_H

#include "details/gc_garbage_collector.h"

namespace precisegc {

inline void gc()
{
    auto& collector = details::gc_garbage_collector::instance();
    collector.start_gc();
    collector.wait_for_gc_finished();
}

}

#endif //DIPLOMA_GC_H
