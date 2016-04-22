#ifndef DIPLOMA_GC_H
#define DIPLOMA_GC_H

#include "details/gc_garbage_collector.h"

namespace precisegc {

inline void gc()
{
    auto& collector = details::gc_garbage_collector::instance();
    collector.start_marking();
    collector.compact();
}

}

#endif //DIPLOMA_GC_H
