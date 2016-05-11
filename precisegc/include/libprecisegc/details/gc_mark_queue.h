#ifndef DIPLOMA_GC_MARK_QUEUE_H
#define DIPLOMA_GC_MARK_QUEUE_H

#include <queue>
#include <memory>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "gc_untyped_ptr.h"
#include "mutex.h"
#include "util.h"

namespace precisegc { namespace details {

class gc_mark_queue: public noncopyable, public nonmovable
{
public:

    static gc_mark_queue& instance();

    bool empty();

    bool push(void* ptr);
    bool pop(void*& p);

    void clear();

private:
    gc_mark_queue();

    boost::lockfree::queue<void*, boost::lockfree::fixed_sized<false>> m_queue;
};

}}

#endif //DIPLOMA_GC_MARK_QUEUE_H
