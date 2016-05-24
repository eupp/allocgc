#ifndef DIPLOMA_GC_MARK_QUEUE_H
#define DIPLOMA_GC_MARK_QUEUE_H

#include <queue>
#include <memory>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "gc_untyped_ptr.h"
#include "libprecisegc/details/utils/utility.hpp"

namespace precisegc { namespace details {

class barrier_buffer: public utils::noncopyable, public utils::nonmovable
{
public:
    barrier_buffer();

    bool push(void* ptr);
    bool pop(void*& p);
private:
    static const size_t MAX_SIZE = 65536;

    boost::lockfree::allocator<std::allocator<void*>> m_alloc;
    boost::lockfree::spsc_queue<void*, boost::lockfree::allocator<std::allocator<void*>>> m_queue;
};

}}

#endif //DIPLOMA_GC_MARK_QUEUE_H
