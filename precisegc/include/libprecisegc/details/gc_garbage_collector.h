#ifndef DIPLOMA_GC_GARBAGE_COLLECTOR_H
#define DIPLOMA_GC_GARBAGE_COLLECTOR_H

#include <pthread.h>
#include <queue>

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

    void start_gc();
    void wait_for_gc_finished();

    void start_marking();
    void wait_for_marking_finished();

    void start_compacting();
    void wait_for_compacting_finished();

    // very bad design, just for testing purpose
    void force_move_to_idle();
    void force_move_to_no_gc();

    size_t get_gc_cycles_count() const;

    void write_barrier(gc_untyped_ptr& dst_ptr, const gc_untyped_ptr& src_ptr);
    void new_cell(managed_cell_ptr& cell_ptr);

private:
    gc_garbage_collector();

    static void* start_marking_routine(void*);
    static void* start_compacting_routine(void*);
    static void mark();
    static void traverse(precisegc::details::managed_cell_ptr root);

    void queue_push(void* p);
    void* queue_pop();
    bool queue_empty();
    void clear_queue();

    enum class phase {
        IDLE,
        MARKING,
        MARKING_FINISHED,
        COMPACTING,
        COMPACTING_FINISHED,
        GC_OFF
    };

    static const char* phase_str(phase ph);

    void pin_objects();
    void unpin_objects();

    bool m_gc_thread_launch;
    pthread_t m_gc_thread;

    typedef mutex mutex_type;

    phase m_phase;
    mutex_type m_phase_mutex;
    condition_variable m_phase_cond;
    size_t m_gc_cycles_cnt;
    std::queue<void*> m_queue;
};

}}

#endif // DIPLOMA_GC_GARBAGE_COLLECTOR_H
