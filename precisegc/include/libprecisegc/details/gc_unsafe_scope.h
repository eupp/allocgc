#ifndef DIPLOMA_GC_CONSISTENCY_H
#define DIPLOMA_GC_CONSISTENCY_H

#include <libprecisegc/details/threads/posix_signal.hpp>

namespace precisegc { namespace details {

class gc_unsafe_scope
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

}}

#endif //DIPLOMA_GC_CONSISTENCY_H
