#ifndef _DIPLOMA_DEREF_ROOTS_H_
#define _DIPLOMA_DEREF_ROOTS_H_

#include <stddef.h>

void register_dereferenced_root(void*, size_t);
void mark_dereferenced_root(void*, bool);
void sweep_dereferenced_roots();
#endif //_DIPLOMA_DEREF_ROOTS_H_
