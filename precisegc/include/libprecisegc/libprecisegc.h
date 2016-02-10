#pragma once

#include "gc_init.h"
#include "gc_new.h"
#include "gc_ptr.h"
#include "go.h"

extern void register_object (void *);
extern void unregister_object (void *);