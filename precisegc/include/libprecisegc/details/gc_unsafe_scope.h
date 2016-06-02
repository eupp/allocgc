#ifndef DIPLOMA_GC_CONSISTENCY_H
#define DIPLOMA_GC_CONSISTENCY_H

#include <libprecisegc/details/threads/posix_signal.hpp>

namespace precisegc { namespace details {

class gc_unsafe_scope
{
public:
    gc_unsafe_scope()
    {
        static threads::posix_signal& sig = threads::posix_signal::instance();
        sig.lock();
    }

    ~gc_unsafe_scope()
    {
        static threads::posix_signal& sig = threads::posix_signal::instance();
        sig.unlock();
    }
};

}}

#endif //DIPLOMA_GC_CONSISTENCY_H
