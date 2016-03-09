#ifndef DIPLOMA_LOCKABLE_RANGE_H
#define DIPLOMA_LOCKABLE_RANGE_H

#include <utility>

namespace precisegc { namespace details { namespace allocators {

template<typename Range, typename Lock>
class lockable_range
{
public:
    typedef typename Range::iterator_type iterator_type;

    lockable_range(const iterator_type& b, const iterator_type& e)
            : m_range(b, e)
    {}

    lockable_range(const iterator_type& b, const iterator_type& e, Lock&& lock)
        : m_range(b, e)
        , m_lock(std::move(lock))
    {}

    iterator_type begin() const
    {
        return m_range.begin();
    }

    iterator_type end() const
    {
        return m_range.end();
    }

    void lock()
    {
        m_lock.lock();
    }

    void unlock()
    {
        m_lock.unlock();
    }
private:
    Range m_range;
    Lock m_lock;
};

}}}

#endif //DIPLOMA_LOCKABLE_RANGE_H
