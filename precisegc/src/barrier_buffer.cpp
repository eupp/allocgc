#include "barrier_buffer.h"

#include "managed_ptr.h"

#include "logging.h"

namespace precisegc { namespace details {

barrier_buffer::barrier_buffer()
    : m_queue(MAX_SIZE)
{}

bool barrier_buffer::push(void* ptr)
{
    if (ptr) {
        managed_cell_ptr cell_ptr(managed_ptr((byte*) ptr), 0);
        if (!cell_ptr.get_mark()) {
            return m_queue.push(ptr);
        }
    }
    return true;
}

bool barrier_buffer::pop(void*& p)
{
    return m_queue.pop(p);
}

}}