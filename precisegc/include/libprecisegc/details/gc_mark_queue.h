#ifndef DIPLOMA_GC_MARK_QUEUE_H
#define DIPLOMA_GC_MARK_QUEUE_H

#include <queue>

#include "gc_untyped_ptr.h"
#include "mutex.h"
#include "util.h"

namespace precisegc { namespace details {

class gc_mark_queue: public noncopyable, public nonmovable
{
public:

    static gc_mark_queue& instance();

    bool empty() const;

    void push(void* ptr);
    void* pop();

    void clear();

private:
    gc_mark_queue() = default;

    mutable mutex m_mutex;
    std::queue<void*> m_queue;
};

}}

#endif //DIPLOMA_GC_MARK_QUEUE_H
