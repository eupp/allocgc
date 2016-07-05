#ifndef DIPLOMA_GC_UNSAFE_SCOPE_HPP
#define DIPLOMA_GC_UNSAFE_SCOPE_HPP

#include <libprecisegc/details/threads/posix_signal.hpp>

#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details {

class gc_unsafe_scope
{
public:
    gc_unsafe_scope()
    {
        logging::debug() << "Thread enters unsafe scope";
        threads::posix_signal::lock();
    }

    ~gc_unsafe_scope()
    {
        logging::debug() << "Thread leaves unsafe scope";
        threads::posix_signal::unlock();
    }

    static void enter_safepoint()
    {
        logging::debug() << "Thread enters safepoint";
        threads::posix_signal::enter_safe_scope();
    }

    static void leave_safepoint()
    {
        logging::debug() << "Thread leaves safepoint";
        threads::posix_signal::leave_safe_scope();
    }
};

}}

#endif //DIPLOMA_GC_UNSAFE_SCOPE_HPP
