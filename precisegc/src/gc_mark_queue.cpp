#include "gc_mark_queue.h"

#include "logging.h"

namespace precisegc { namespace details {

gc_mark_queue::gc_mark_queue()
    : m_queue(MAX_SIZE)
{}

//gc_mark_queue& gc_mark_queue::instance()
//{
//    static gc_mark_queue queue;
//    return queue;
//}

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
    return m_queue.pop(p);
}

void gc_mark_queue::clear()
{
    void* ret;
    while (m_queue.pop(ret)) {}
}
}}