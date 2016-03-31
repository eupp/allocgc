#include "gc_mark_queue.h"

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

void gc_mark_queue::push(void* ptr)
{
    bool res = m_queue.push(ptr);
    assert(res);
}

void* gc_mark_queue::pop()
{
    void* ret;
    if (!m_queue.pop(ret)) {
        ret = nullptr;
    }
    return ret;
}

void gc_mark_queue::clear()
{
    void* ret;
    while (m_queue.pop(ret)) {}
}
}}