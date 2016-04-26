#ifndef DIPLOMA_GC_CONSISTENCY_H
#define DIPLOMA_GC_CONSISTENCY_H

#include "gc_pause.h"
#include "mutex.h"
#include "logging.h"

namespace precisegc { namespace details {

class gc_unsafe_scope
{
    typedef gc_pause_lock::siglock siglock;
public:
    gc_unsafe_scope()
    {
//        logging::debug() << "Thread " << pthread_self() << " enters unsafe scope (gc signal is disabled)";
        siglock::lock();
    }

    ~gc_unsafe_scope()
    {
//        logging::debug() << "Thread " << pthread_self() << " leaves unsafe scope (gc signal is enabled)";
        siglock::unlock();
    }
};

}
}

#endif //DIPLOMA_GC_CONSISTENCY_H
