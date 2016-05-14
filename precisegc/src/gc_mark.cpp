#include "gcmalloc.h"

#include "gc_mark.h"

#include <cassert>

#include <libprecisegc/details/threads/managed_thread.hpp>

#include "details/managed_ptr.h"
#include "details/barrier_buffer.h"
#include "thread.h"

namespace precisegc { namespace details {

void set_object_pin(void* ptr, bool pinned)
{
    assert(ptr);
    managed_cell_ptr cell_ptr(managed_ptr(reinterpret_cast<byte*>(ptr)), 0);
//    cell_ptr.lock_descriptor();
    cell_ptr.set_pin(pinned);
}

void set_object_mark(void* ptr, bool marked)
{
    assert(ptr);
    managed_cell_ptr cell_ptr(managed_ptr(reinterpret_cast<byte*>(ptr)), 0);
//    cell_ptr.lock_descriptor();
    cell_ptr.set_mark(marked);
}

bool get_object_pin(void* ptr)
{
    assert(ptr);
    managed_cell_ptr cell_ptr(managed_ptr(reinterpret_cast<byte*>(ptr)), 0);
//    cell_ptr.lock_descriptor();
    return cell_ptr.get_pin();
}

bool get_object_mark(void* ptr)
{
    assert(ptr);
    managed_cell_ptr cell_ptr(managed_ptr(reinterpret_cast<byte*>(ptr)), 0);
//    cell_ptr.lock_descriptor();
    return cell_ptr.get_mark();
}

bool shade(const gc_untyped_ptr* ptr)
{
    static thread_local barrier_buffer& bb = threads::managed_thread::this_thread().get_barrier_buffer();
    if (!ptr) {
        return true;
    }
    return bb.push((gc_untyped_ptr*) ptr);
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