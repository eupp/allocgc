#include "gc_mark_queue.h"

#include "logging.h"

namespace precisegc { namespace details {

gc_mark_queue& gc_mark_queue::instance()
{
    static gc_mark_queue queue;
    return queue;
}

bool gc_mark_queue::empty() const
{
    lock_guard<mutex> lock(m_mutex);
    return m_queue.empty();
}

void gc_mark_queue::push(void* ptr)
{
    lock_guard<mutex> lock(m_mutex);
    m_queue.push(ptr);
}

void* gc_mark_queue::pop()
{
    lock_guard<mutex> lock(m_mutex);
    void* ret = m_queue.front();
    m_queue.pop();
    return ret;
}

void gc_mark_queue::clear()
{
    lock_guard<mutex> lock(m_mutex);
    std::queue<void*>().swap(m_queue);
}

}}