#ifndef DIPLOMA_LOCKED_RANGE_HPP
#define DIPLOMA_LOCKED_RANGE_HPP

#include <mutex>
#include <utility>
#include <type_traits>

#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace utils {

template <typename Range, typename Lockable>
class locked_range : public Range, private utils::noncopyable
{
public:
    typedef typename Range::iterator iterator;

    template<typename R, typename = typename std::enable_if<
            std::is_same<Range, typename std::remove_cv<typename std::remove_reference<R>::type>::type>::value
        >::type>
    locked_range(R&& range, std::unique_lock<Lockable> lock)
        : Range(std::forward<R>(range))
        , m_lock(std::move(lock))
    {}

    locked_range(locked_range&& other) = default;

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

}}}

#endif //DIPLOMA_LOCKED_RANGE_HPP
