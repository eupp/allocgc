#ifndef DIPLOMA_GC_GARBAGE_COLLECTOR_H
#define DIPLOMA_GC_GARBAGE_COLLECTOR_H

#include <pthread.h>

#include "mutex.h"
#include "condition_variable.h"
#include "util.h"

namespace precisegc { namespace details {

class gc_garbage_collector: public noncopyable, public nonmovable
{
public:

    static gc_garbage_collector& instance();

    void start_gc();
    void wait_for_gc_finished();

    void start_marking();
    void wait_for_marking_finished();

    void start_compacting();
    void wait_for_compacting_finished();

    // very bad design, just for testing purpose
    void force_move_to_idle();

private:
    gc_garbage_collector();

    static void* start_marking_routine(void*);
    static void* start_compacting_routine(void*);
    static void mark();
    static void traverse(void* root);

    enum class phase {
        IDLE,
        MARKING,
        MARKING_FINISHED,
        COMPACTING,
        COMPACTING_FINISHED
    };

    static const char* phase_str(phase ph);

    pthread_t m_marking_thread;
    pthread_t m_gc_thread;

    phase m_phase;
    mutex m_phase_mutex;
    condition_variable m_phase_cond;

};

}}

#endif // DIPLOMA_GC_GARBAGE_COLLECTOR_H
