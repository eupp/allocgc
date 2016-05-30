#ifndef DIPLOMA_GC_GARBAGE_COLLECTOR_H
#define DIPLOMA_GC_GARBAGE_COLLECTOR_H

#include <pthread.h>
#include <queue>
#include <atomic>

#include <libprecisegc/details/threads/thread_manager.hpp>

#include "marker.hpp"
#include "libprecisegc/details/ptrs/gc_untyped_ptr.hpp"
#include "libprecisegc/details/utils/utility.hpp"
#include "managed_ptr.hpp"

namespace precisegc { namespace details {

class gc_garbage_collector: public utils::noncopyable, public utils::nonmovable
{
public:
    static gc_garbage_collector& instance();

    ~gc_garbage_collector();

    void start_marking();
    void start_compacting();

    // very bad design, just for testing purpose
    void force_move_to_idle();

    size_t get_gc_cycles_count() const;

    void write_barrier(ptrs::gc_untyped_ptr& dst_ptr, const ptrs::gc_untyped_ptr& src_ptr);
    void new_cell(managed_ptr& cell_ptr);
private:
    static threads::thread_manager& thread_manager;

    gc_garbage_collector();

    void run_marking();
    void run_compacting();
    void run_gc();

    enum class phase {
        IDLE,
        MARKING,
        COMPACTING
    };

    static const char* phase_str(phase ph);

    size_t m_gc_cycles_cnt;
    std::atomic<phase> m_phase;
    marker m_marker;
};

}}

#endif // DIPLOMA_GC_GARBAGE_COLLECTOR_H
