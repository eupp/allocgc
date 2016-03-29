#pragma once

#include <cstdlib>

#include "details/object_meta.h"

namespace _GC_ {

    class base_meta {
    public:
        size_t *shell; /**< pointer on the box(meta info struct for storing offsets) of object */
        size_t count;  /**< array count */
    };

    bool is_heap_pointer(void *);

    void* gcmalloc(size_t, const precisegc::details::class_meta* , size_t);

    void set_meta_after_gcmalloc(void *ptr, const precisegc::details::class_meta *cls_meta);

base_meta * get_meta_inf (void * ptr);

    bool get_object_mark(void* ptr);
    bool get_object_pin(void* ptr);

    void set_object_mark(void* ptr, bool marked);
    void set_object_pin(void* ptr, bool pinned);

    int mark_object(void *ptr, bool mark_pin);
    int get_object_mark(void *ptr, bool mark_pin);

    bool mark_after_overflow(void);
}