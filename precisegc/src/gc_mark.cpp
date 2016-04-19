#include "gcmalloc.h"
#include "gc_mark.h"

#include <cassert>

#include "details/managed_ptr.h"
#include "details/gc_mark_queue.h"

namespace precisegc { namespace details {

void set_object_pin(void* ptr, bool pinned)
{
    assert(ptr);
    managed_cell_ptr cell_ptr(managed_ptr(reinterpret_cast<byte*>(ptr)), 0);
    cell_ptr.lock_descriptor();
    cell_ptr.set_pin(pinned);
}

void set_object_mark(void* ptr, bool marked)
{
    assert(ptr);
    managed_cell_ptr cell_ptr(managed_ptr(reinterpret_cast<byte*>(ptr)), 0);
    cell_ptr.lock_descriptor();
    cell_ptr.set_mark(marked);
}

bool get_object_pin(void* ptr)
{
    assert(ptr);
    managed_cell_ptr cell_ptr(managed_ptr(reinterpret_cast<byte*>(ptr)), 0);
    cell_ptr.lock_descriptor();
    return cell_ptr.get_pin();
}

bool get_object_mark(void* ptr)
{
    assert(ptr);
    managed_cell_ptr cell_ptr(managed_ptr(reinterpret_cast<byte*>(ptr)), 0);
    cell_ptr.lock_descriptor();
    return cell_ptr.get_mark();
}

void shade(void* ptr)
{
    static gc_mark_queue& queue = gc_mark_queue::instance();
    queue.push(ptr);
//    if (!ptr) {
//        return;
//    }
//    managed_cell_ptr cell_ptr(managed_ptr(reinterpret_cast<byte*>(ptr)), 0);
//    // this check will be failed only when ptr is pointed to non gc_heap memory,
//    // that is not possible in correct program (i.e. when gc_new is used to create managed objects),
//    // but could occur during testing.
//    try {
//        cell_ptr.lock_descriptor();
//        if (!cell_ptr.get_mark()) {
//            static gc_mark_queue& queue = gc_mark_queue::instance();
//            queue.push(ptr);
//        }
//    } catch (managed_cell_ptr::unindexed_memory_exception& exc) {
//        return;
//    }
}

void* get_pointed_to(void* ptr)
{
    gc_untyped_ptr* gcptr = reinterpret_cast<gc_untyped_ptr*>(ptr);
    void* res = gcptr->get();
    return res;
}

object_meta* get_object_header(void *ptr) {
    managed_cell_ptr cell_ptr(managed_ptr(reinterpret_cast<byte*>(ptr)), 0);
    return cell_ptr.get_meta();
}

}}