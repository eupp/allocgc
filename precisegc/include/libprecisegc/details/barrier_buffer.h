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

class barrier_buffer: public noncopyable, public nonmovable
{
public:
    barrier_buffer();

    bool push(gc_untyped_ptr* ptr);
    bool pop(gc_untyped_ptr*& p);
private:
    static const size_t MAX_SIZE = 65536;

    boost::lockfree::allocator<std::allocator<void*>> m_alloc;
    boost::lockfree::spsc_queue<gc_untyped_ptr*, boost::lockfree::allocator<std::allocator<void*>>> m_queue;
};

}}

#endif //DIPLOMA_GC_MARK_QUEUE_H
