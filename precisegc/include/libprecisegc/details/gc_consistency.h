#ifndef DIPLOMA_GC_CONSISTENCY_H
#define DIPLOMA_GC_CONSISTENCY_H

#include "gc_pause.h"
#include "mutex.h"

namespace precisegc { namespace details {

class gc_consistency_lock
{
public:
    gc_consistency_lock()
        : m_lock_guard(m_pause_lock)
    {}
private:
    gc_pause_lock m_pause_lock;
    lock_guard<gc_pause_lock> m_lock_guard;
};

}
}

#endif //DIPLOMA_GC_CONSISTENCY_H
