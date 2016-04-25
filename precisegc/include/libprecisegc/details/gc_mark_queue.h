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

//    static gc_mark_queue& instance();

    gc_mark_queue();

    bool empty();
    bool push(void* ptr);

    bool pop(void*& p);

    void clear();
private:

    static const size_t MAX_SIZE = 4096;

    boost::lockfree::allocator<std::allocator<void*>> m_alloc;
    boost::lockfree::spsc_queue<void*, boost::lockfree::allocator<std::allocator<void*>>> m_queue;
};

}}

#endif //DIPLOMA_GC_MARK_QUEUE_H
