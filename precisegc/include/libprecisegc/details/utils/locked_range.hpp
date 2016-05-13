#ifndef DIPLOMA_LOCKED_RANGE_HPP
#define DIPLOMA_LOCKED_RANGE_HPP

#include <mutex>

namespace precisegc { namespace details { namespace utils {

template <typename Range, typename Lockable>
class locked_range : public Range
{
public:
    typedef typename Range::iterator iterator;

    locked_range(const Range& r, Lockable& lockable)
        : Range(r)
        , m_lock(lockable)
    {}
private:
    std::unique_lock<Lockable> m_lock;
};

}}}

#endif //DIPLOMA_LOCKED_RANGE_HPP
