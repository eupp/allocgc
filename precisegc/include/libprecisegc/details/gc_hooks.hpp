#ifndef DIPLOMA_GC_HOOKS_HPP
#define DIPLOMA_GC_HOOKS_HPP

#include <memory>

#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/managed_ptr.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/logging.h>

namespace precisegc { namespace details {

gc_interface* gc_get_strategy();
void gc_set_strategy(std::unique_ptr<gc_interface> strategy);
std::unique_ptr<gc_interface> gc_reset_strategy(std::unique_ptr<gc_interface> strategy);

managed_ptr gc_allocate(size_t size);

byte* gc_rbarrier(const atomic_byte_ptr& p);
void  gc_wbarrier(atomic_byte_ptr& dst, const atomic_byte_ptr& src);

void gc_initation_point(initation_point_type ipoint);

void gc_enable_printer();
void gc_disable_printer();

void register_gc(const gc_sweep_stat& sweep_stat, const gc_pause_stat& pause_stat);
void register_pause(const gc_pause_stat& pause_stat);

}}

#endif //DIPLOMA_GC_HOOKS_HPP
