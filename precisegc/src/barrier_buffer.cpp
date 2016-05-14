#include "barrier_buffer.h"

#include "managed_ptr.h"

#include "logging.h"

namespace precisegc { namespace details {

barrier_buffer::barrier_buffer()
    : m_queue(MAX_SIZE)
{}

bool barrier_buffer::push(gc_untyped_ptr* ptr)
{
    return m_queue.push(ptr);
}

bool barrier_buffer::pop(gc_untyped_ptr*& p)
{
    p = nullptr;
    gc_untyped_ptr* ptr = nullptr;
    if (m_queue.pop(ptr)) {
        void* obj = ptr->get();
        if (obj) {
            managed_cell_ptr cell_ptr(managed_ptr((byte*) obj), 0);
            if (!cell_ptr.get_mark()) {
                p = ptr;
            }
        }
        return true;
    }
    return false;
}

}}