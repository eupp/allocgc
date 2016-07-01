#ifndef DIPLOMA_DUMMY_MUTEX_HPP
#define DIPLOMA_DUMMY_MUTEX_HPP

#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace utils {

class dummy_mutex : noncopyable, nonmovable
{
public:
    dummy_mutex() = default;

    void lock() {}
    void unlock() {}
    bool try_lock() { return true; }
};

}}}

#endif //DIPLOMA_DUMMY_MUTEX_HPP
