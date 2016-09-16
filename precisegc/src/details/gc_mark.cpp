#include "gc_mark.hpp"

#include <cassert>

#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/logging.hpp>

#include "details/managed_ptr.hpp"

namespace precisegc { namespace details {

void set_object_pin(void* ptr, bool pinned)
{
    assert(ptr);
    managed_ptr cell_ptr(reinterpret_cast<byte*>(ptr));
//    cell_ptr.lock_descriptor();
    cell_ptr.set_pin(pinned);
}

void set_object_mark(void* ptr, bool marked)
{
    assert(ptr);
    managed_ptr cell_ptr(reinterpret_cast<byte*>(ptr));
//    cell_ptr.lock_descriptor();
    cell_ptr.set_mark(marked);
}

bool get_object_pin(void* ptr)
{
    assert(ptr);
    managed_ptr cell_ptr(reinterpret_cast<byte*>(ptr));
//    cell_ptr.lock_descriptor();
    return cell_ptr.get_pin();
}

bool get_object_mark(void* ptr)
{
    assert(ptr);
    managed_ptr cell_ptr(reinterpret_cast<byte*>(ptr));
//    cell_ptr.lock_descriptor();
    return cell_ptr.get_mark();
}

object_meta* get_object_header(void *ptr) {
    managed_ptr cell_ptr(reinterpret_cast<byte*>(ptr));
    return cell_ptr.get_meta();
}

}}