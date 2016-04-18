#ifndef DIPLOMA_WRITE_BARRIER_H
#define DIPLOMA_WRITE_BARRIER_H

#include "gc_garbage_collector.h"
#include "gc_untyped_ptr.h"
#include "gc_mark.h"

namespace precisegc { namespace details {

inline void write_barrier(gc_untyped_ptr& dst_ptr, const gc_untyped_ptr& src_ptr)
{
    static auto& collector = gc_garbage_collector::instance();
    collector.write_barrier(dst_ptr, src_ptr);
}

}}

#endif //DIPLOMA_WRITE_BARRIER_H
