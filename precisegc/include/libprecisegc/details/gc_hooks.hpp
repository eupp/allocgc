#ifndef DIPLOMA_GC_HOOKS_HPP
#define DIPLOMA_GC_HOOKS_HPP

#include <libprecisegc/details/managed_ptr.hpp>
#include <libprecisegc/details/gc_untyped_ptr.h>


namespace precisegc { namespace details {

enum class gc_strategy {
    INCREMENTAL
};

enum class gc_phase {
    IDLE,
    MARKING,
    COMPACTING
};

void gc_init(gc_strategy strategy);

gc_phase gc_get_phase();

managed_ptr gc_allocate(size_t size);

void gc_write_barrier(gc_untyped_ptr& dst, const gc_untyped_ptr& src);

byte* gc_load(const atomic_byte_ptr& p);
void  gc_store(atomic_byte_ptr& p, byte* value);

}}

#endif //DIPLOMA_GC_HOOKS_HPP
