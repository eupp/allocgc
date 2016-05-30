#ifndef DIPLOMA_GC_HOOKS_HPP
#define DIPLOMA_GC_HOOKS_HPP

#include <memory>

#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/managed_ptr.hpp>
#include <libprecisegc/details/gc_untyped_ptr.h>
#include <libprecisegc/details/logging.h>

namespace precisegc { namespace details {

enum class gc_strategy {
      SERIAL
    , INCREMENTAL
};

enum class gc_compacting {
      ENABLED
    , DISABLED
};

void gc_set(std::unique_ptr<gc_interface>&& gc);
std::unique_ptr<gc_interface> gc_reset(std::unique_ptr<gc_interface>&& gc);

managed_ptr gc_allocate(size_t size);

byte* gc_rbarrier(const atomic_byte_ptr& p);
void  gc_wbarrier(atomic_byte_ptr& dst, const atomic_byte_ptr& src);

}}

#endif //DIPLOMA_GC_HOOKS_HPP
