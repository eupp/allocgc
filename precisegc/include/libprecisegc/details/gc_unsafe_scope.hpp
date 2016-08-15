#ifndef DIPLOMA_GC_UNSAFE_SCOPE_HPP
#define DIPLOMA_GC_UNSAFE_SCOPE_HPP

#include <libprecisegc/details/threads/posix_signal.hpp>

#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details {

class gc_unsafe_scope : private utils::noncopyable, private utils::nonmovable
{
public:
    gc_unsafe_scope()
    {
        threads::posix_signal::lock();
    }

    ~gc_unsafe_scope()
    {
        threads::posix_signal::unlock();
    }
};

class gc_unsafe_scope_lock : private utils::noncopyable, private utils::nonmovable
{
public:
    gc_unsafe_scope_lock() = default;

    void lock()
    {
        threads::posix_signal::lock();
    }

    void unlock()
    {
        threads::posix_signal::unlock();
    }
};

class gc_safe_scope : private utils::noncopyable, private utils::nonmovable
{
public:
    gc_safe_scope()
    {
        threads::posix_signal::enter_safe_scope();
    }

    ~gc_safe_scope()
    {
        threads::posix_signal::leave_safe_scope();
    }
};

class gc_safe_scope_lock : private utils::noncopyable, private utils::nonmovable
{
public:
    gc_safe_scope_lock() = default;

    void lock()
    {
        threads::posix_signal::enter_safe_scope();
    }

    void unlock()
    {
        threads::posix_signal::leave_safe_scope();
    }
};

}}

#endif //DIPLOMA_GC_UNSAFE_SCOPE_HPP
