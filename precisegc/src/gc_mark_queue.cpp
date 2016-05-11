#include "gc_mark_queue.h"

#include "managed_ptr.h"

#include "logging.h"

namespace precisegc { namespace details {

gc_mark_queue::gc_mark_queue()
    : m_queue(0)
{}

gc_mark_queue& gc_mark_queue::instance()
{
    static gc_mark_queue queue;
    return queue;
}

bool gc_mark_queue::empty()
{
    return m_queue.empty();
}

bool gc_mark_queue::push(void* ptr)
{
    return m_queue.push(ptr);
}

bool gc_mark_queue::pop(void*& p)
{
//    p = nullptr;
//    void* ptr = nullptr;
//    if (m_queue.pop(ptr)) {
//        managed_cell_ptr cell_ptr(managed_ptr(reinterpret_cast<byte*>(ptr)), 0);
//        if (!cell_ptr.get_mark()) {
//            p = ptr;
//        }
//        return true;
//    }
//    return false;

//    // this check will be failed only when ptr is pointed to non gc_heap memory,
//    // that is not possible in correct program (i.e. when gc_new is used to create managed objects),
//    // but could occur during testing.
//    try {
//        cell_ptr.lock_descriptor();
//        if (!cell_ptr.get_mark()) {
////            static gc_mark_queue& queue = gc_mark_queue::instance();
//            queue->push(ptr);
//        }
//    } catch (managed_cell_ptr::unindexed_memory_exception& exc) {
//        return;
//    }
    return m_queue.pop(p);
}

void gc_mark_queue::clear()
{
    void* ret;
    while (m_queue.pop(ret)) {}
}
}}