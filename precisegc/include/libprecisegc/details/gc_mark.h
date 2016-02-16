#ifndef DIPLOMA_GC_MARK_H
#define DIPLOMA_GC_MARK_H

namespace precisegc { namespace details {

void shade(void* ptr);

bool get_object_mark(void* ptr);
bool get_object_pin(void* ptr);

void set_object_mark(void* ptr, bool marked);
void set_object_pin(void* ptr, bool pinned);

void* get_pointed_to(void* ptr);

}}

#endif //DIPLOMA_GC_MARK_H
