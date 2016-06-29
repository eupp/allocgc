#ifndef DIPLOMA_GC_MARK_QUEUE_H
#define DIPLOMA_GC_MARK_QUEUE_H

#include <queue>
#include <memory>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "libprecisegc/details/ptrs/gc_untyped_ptr.hpp"
#include "libprecisegc/details/utils/utility.hpp"

namespace precisegc { namespace details {

class barrier_buffer: public utils::noncopyable, public utils::nonmovable
{
public:
    barrier_buffer();

    bool push(void* ptr);
    bool pop(void*& p);

    template <typename Functor>
    void trace(Functor&& f)
    {
        m_queue.consume_all(f);
    }
private:
    static const size_t MAX_SIZE = 65536;

    boost::lockfree::queue<void*, boost::lockfree::fixed_sized<false>> m_queue;
};

}}

#endif //DIPLOMA_GC_MARK_QUEUE_H