#ifndef DIPLOMA_GC_HOOKS_HPP
#define DIPLOMA_GC_HOOKS_HPP

#include <libprecisegc/details/managed_ptr.h>
#include <libprecisegc/details/gc_untyped_ptr.h>

namespace precisegc { namespace details {



class gc_hooks
{
public:
    virtual managed_ptr allocate(size_t size) = 0;
    virtual void write_barrier(gc_untyped_ptr* dst, gc_untyped_ptr* src) = 0;
};

}}

#endif //DIPLOMA_GC_HOOKS_HPP
