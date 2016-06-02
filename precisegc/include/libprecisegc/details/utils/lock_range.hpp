#ifndef DIPLOMA_LOCKED_RANGE_HPP
#define DIPLOMA_LOCKED_RANGE_HPP

#include <mutex>
#include <utility>

#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace utils {

template <typename Range, typename Lockable>
class locked_range : public Range, private utils::noncopyable
{
public:
    typedef typename Range::iterator iterator;

    locked_range(Range& r, Lockable& lockable)
        : Range(r)
        , m_lock(lockable)
    {}

    locked_range(Range& r, Lockable& lockable, std::defer_lock_t)
        : Range(r)
        , m_lock(lockable, std::defer_lock)
    {}

    locked_range(Range& r, Lockable& lockable, std::try_to_lock_t)
        : Range(r)
        , m_lock(lockable, std::try_to_lock)
    {}

    locked_range(Range& r, Lockable& lockable, std::adopt_lock_t)
        : Range(r)
        , m_lock(lockable, std::adopt_lock)
    {}

    locked_range(const Range& r, Lockable& lockable)
        : Range(r)
        , m_lock(lockable)
    {}

    locked_range(const Range& r, Lockable& lockable, std::defer_lock_t)
        : Range(r)
        , m_lock(lockable, std::defer_lock)
    {}

    locked_range(const Range& r, Lockable& lockable, std::try_to_lock_t)
        : Range(r)
        , m_lock(lockable, std::try_to_lock)
    {}

    locked_range(const Range& r, Lockable& lockable, std::adopt_lock_t)
        : Range(r)
        , m_lock(lockable, std::adopt_lock)
    {}

    locked_range(locked_range&& other)
        : Range(other)
        , m_lock(std::move(other.m_lock))
    {}

    bool owns_lock() const
    {
        return m_lock.owns_lock();
    }

    void lock()
    {
        assert(!m_lock.owns_lock());
        m_lock.lock();
    }

    void unlock()
    {
        assert(m_lock.owns_lock());
        m_lock.unlock();
    }
private:
    std::unique_lock<Lockable> m_lock;
};

template <typename Range, typename Lockable>
locked_range<Range, Lockable> lock_range(Range& range, Lockable& lockable)
{
    return locked_range<Range, Lockable>(range, lockable);
};

template <typename Range, typename Lockable>
locked_range<Range, Lockable> lock_range(Range& range, Lockable& lockable, std::defer_lock_t)
{
    return locked_range<Range, Lockable>(range, lockable, std::defer_lock);
};

template <typename Range, typename Lockable>
locked_range<Range, Lockable> lock_range(Range& range, Lockable& lockable, std::try_to_lock_t)
{
    return locked_range<Range, Lockable>(range, lockable, std::try_to_lock);
};

template <typename Range, typename Lockable>
locked_range<Range, Lockable> lock_range(Range& range, Lockable& lockable, std::adopt_lock_t)
{
    return locked_range<Range, Lockable>(range, lockable, std::adopt_lock);
};

template <typename Range, typename Lockable>
locked_range<Range, Lockable> lock_range(const Range& range, Lockable& lockable)
{
    return locked_range<Range, Lockable>(range, lockable);
};

template <typename Range, typename Lockable>
locked_range<Range, Lockable> lock_range(const Range& range, Lockable& lockable, std::defer_lock_t)
{
    return locked_range<Range, Lockable>(range, lockable, std::defer_lock);
};

template <typename Range, typename Lockable>
locked_range<Range, Lockable> lock_range(const Range& range, Lockable& lockable, std::try_to_lock_t)
{
    return locked_range<Range, Lockable>(range, lockable, std::try_to_lock);
};

template <typename Range, typename Lockable>
locked_range<Range, Lockable> lock_range(const Range& range, Lockable& lockable, std::adopt_lock_t)
{
    return locked_range<Range, Lockable>(range, lockable, std::adopt_lock);
};

}}}

#endif //DIPLOMA_LOCKED_RANGE_HPP
