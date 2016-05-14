#include "gc_mark_queue.h"

#include "managed_ptr.h"

#include "logging.h"

namespace precisegc { namespace details {

gc_mark_queue::gc_mark_queue()
    : m_queue(MAX_SIZE)
{}

bool gc_mark_queue::push(void* ptr)
{
    return m_queue.push(ptr);
}

bool gc_mark_queue::pop(void*& p)
{
    p = nullptr;
    void* ptr = nullptr;
    if (m_queue.pop(ptr)) {
        managed_cell_ptr cell_ptr(managed_ptr(reinterpret_cast<byte*>(ptr)), 0);
        if (!cell_ptr.get_mark()) {
            p = ptr;
            return true;
        }
    }
    return false;
}

}}