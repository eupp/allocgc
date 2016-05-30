#ifndef DIPLOMA_GC_MARK_H
#define DIPLOMA_GC_MARK_H

#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/object_meta.h>

namespace precisegc { namespace details {

bool shade(const ptrs::gc_untyped_ptr* ptr);

bool get_object_mark(void* ptr);
bool get_object_pin(void* ptr);

void set_object_mark(void* ptr, bool marked);
void set_object_pin(void* ptr, bool pinned);

void* get_pointed_to(void* ptr);

object_meta * get_object_header(void *ptr);

}}

#endif //DIPLOMA_GC_MARK_H
