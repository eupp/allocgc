#ifndef DIPLOMA_SAFE_SCOPE_LOCK_HPP
#define DIPLOMA_SAFE_SCOPE_LOCK_HPP

#include <libprecisegc/details/gc_unsafe_scope.hpp>
#include <libprecisegc/details/utils/scope_guard.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace utils {

template <typename Lockable>
class safe_scope_lock : private noncopyable, private nonmovable
{
public:
    safe_scope_lock() = default;

    void lock()
    {
        gc_unsafe_scope::enter_safepoint();
        auto guard = make_scope_guard([] { gc_unsafe_scope::leave_safepoint(); });
        m_lock.lock();
    }

    void unlock()
    {
        m_lock.unlock();
    }

    bool try_lock()
    {
        return m_lock.try_lock();
    }
private:
    Lockable m_lock;
};

}}}

#endif //DIPLOMA_SAFE_SCOPE_LOCK_HPP
