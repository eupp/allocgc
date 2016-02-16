#ifndef DIPLOMA_WRITE_BARRIER_H
#define DIPLOMA_WRITE_BARRIER_H

#include "gc_untyped_ptr.h"
#include "gc_mark.h"

namespace precisegc { namespace details {

void write_barrier(gc_untyped_ptr& dst_ptr, const gc_untyped_ptr& src_ptr)
{
    dst_ptr.atomic_store(src_ptr);
    shade(src_ptr.get());
}

}}

#endif //DIPLOMA_WRITE_BARRIER_H
