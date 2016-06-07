#ifndef DIPLOMA_GC_INTERFACE_HPP
#define DIPLOMA_GC_INTERFACE_HPP

#include <libprecisegc/details/gc_exception.hpp>
#include <libprecisegc/details/gc_clock.hpp>

namespace precisegc { namespace details {

enum class initation_point_type {
      USER_REQUEST
    , AFTER_ALLOC
};

enum class gc_pause_type {
      TRACE_ROOTS
    , SWEEP_HEAP
    , NO_PAUSE
};

enum class gc_phase {
      IDLING
    , MARKING
    , SWEEPING
};

struct gc_info
{
    bool    incremental;
    bool    support_concurrent_mark;
    bool    support_concurrent_sweep;
};

struct gc_stat
{
    size_t              heap_size;
    size_t              heap_gain;
    gc_clock::duration  last_alloc_timediff;
    gc_clock::duration  last_gc_timediff;
    gc_clock::duration  last_gc_duration;
};

struct gc_pause_stat
{
    gc_pause_type       type;
    gc_clock::duration  duration;
};

struct gc_sweep_stat
{
    size_t      shrunk;
    size_t      swept;
};

struct incremental_gc_ops
{
    gc_phase    phase;
    bool        concurrent_flag;
    size_t      threads_num;
};

inline bool operator==(const incremental_gc_ops& a, const incremental_gc_ops& b)
{
    return a.phase == b.phase && a.concurrent_flag == b.concurrent_flag && a.threads_num == b.threads_num;
}

inline bool operator!=(const incremental_gc_ops& a, const incremental_gc_ops& b)
{
    return !(a == b);
}

class gc_bad_alloc : public gc_exception
{
public:
    gc_bad_alloc(const char* msg)
            : gc_exception(std::string("failed to allocate memory; ") + msg)
    {}

    gc_bad_alloc(const std::string& msg)
            : gc_exception("failed to allocate memory; " + msg)
    {}
};

}}

#endif //DIPLOMA_GC_INTERFACE_HPP
