#ifndef ALLOCGC_DUMMY_MUTEX_HPP
#define ALLOCGC_DUMMY_MUTEX_HPP

#include <liballocgc/details/utils/utility.hpp>

namespace allocgc { namespace details { namespace utils {

class dummy_mutex : noncopyable, nonmovable
{
public:
    dummy_mutex() = default;

    void lock() {}
    void unlock() {}
    bool try_lock() { return true; }
};

}}}

#endif //ALLOCGC_DUMMY_MUTEX_HPP
