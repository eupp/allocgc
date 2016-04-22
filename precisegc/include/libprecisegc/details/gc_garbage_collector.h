#ifndef DIPLOMA_GC_GARBAGE_COLLECTOR_H
#define DIPLOMA_GC_GARBAGE_COLLECTOR_H

#include <pthread.h>
#include <queue>
#include <atomic>

#include "gc_untyped_ptr.h"
#include "mutex.h"
#include "condition_variable.h"
#include "util.h"
#include "managed_ptr.h"

namespace precisegc { namespace details {

class gc_garbage_collector: public noncopyable, public nonmovable
{
public:

    static gc_garbage_collector& instance();

    void start_marking();
    void pause_marking();
    void compact();

    // very bad design, just for testing purpose
    void force_move_to_no_gc();
    void force_move_to_idle();

    size_t get_gc_cycles_count() const;

    void write_barrier(gc_untyped_ptr& dst_ptr, const gc_untyped_ptr& src_ptr);
    void new_cell(managed_cell_ptr& cell_ptr);

private:
    gc_garbage_collector();

    static void* marking_routine(void*);
    void compacting_routine();
    static void mark();
    static void traverse(precisegc::details::managed_cell_ptr root);

    void queue_push(void* p);
    void* queue_pop();
    bool queue_empty();
    void clear_queue();

    typedef mutex mutex_type;

    enum class phase {
        IDLE,
        MARKING,
        MARKING_FINISHED,
        COMPACTING,
        GC_OFF
    };

    enum class marking_state {
        REQUESTED,
        RUNNING,
        PAUSE_REQUESTED,
        PAUSED,
        OFF
    };

    static const char* phase_str(phase ph);
    void pin_objects();

    void unpin_objects();

    std::atomic<phase> m_phase;
    mutex_type m_mark_mutex;
    condition_variable m_mark_cond;
    marking_state m_mark_state;
    size_t m_gc_cycles_cnt;
    std::queue<void*> m_queue;
    bool m_mark_thread_launch;
    pthread_t m_mark_thread;
};

}}

#endif // DIPLOMA_GC_GARBAGE_COLLECTOR_H
