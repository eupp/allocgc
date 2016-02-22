#include "gc_mark.h"

#include "details/page_descriptor.h"
#include "details/gc_mark_queue.h"
#include "index_tree.h"

using namespace _GC_;

namespace precisegc { namespace details {

void set_object_pin(void* ptr, bool pinned)
{
    page_descriptor* pd = (page_descriptor*) IT_get_page_descr(ptr);
    lock_guard<mutex> lock(pd->get_bitmap_mutex());
    pd->set_object_pin(ptr, pinned);
}

void set_object_mark(void* ptr, bool marked)
{
    page_descriptor* pd = (page_descriptor*) IT_get_page_descr(ptr);
    lock_guard<mutex> lock(pd->get_bitmap_mutex());
    pd->set_object_mark(ptr, marked);
}

bool get_object_pin(void* ptr)
{
    page_descriptor* pd = (page_descriptor*) IT_get_page_descr(ptr);
    lock_guard<mutex> lock(pd->get_bitmap_mutex());
    return pd->get_object_pin(ptr);
}

bool get_object_mark(void* ptr)
{
    page_descriptor* pd = (page_descriptor*) IT_get_page_descr(ptr);
    lock_guard<mutex> lock(pd->get_bitmap_mutex());
    return pd->get_object_mark(ptr);
}

void shade(void* ptr)
{
    page_descriptor* pd = (page_descriptor*) IT_get_page_descr(ptr);
    // this check will be failed only when ptr is pointed to non gc_heap memory,
    // that is not possible in correct program (i.e. when gc_new is used to create managed objects),
    // but could occur during testing.
    if (pd) {
        lock_guard<mutex> lock(pd->get_bitmap_mutex());
        if (!pd->get_object_mark(ptr)) {
            gc_mark_queue& queue = gc_mark_queue::instance();
            queue.push(ptr);
        }
    }
}

void* get_pointed_to(void* ptr)
{
    void* res = (void*) (* ((uintptr_t*) ptr));
    return res;
}

}}