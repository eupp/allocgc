#ifndef DIPLOMA_GC_INTERFACE_HPP
#define DIPLOMA_GC_INTERFACE_HPP

#include <string>

#include <libprecisegc/details/gc_exception.hpp>
#include <libprecisegc/details/gc_clock.hpp>

namespace precisegc { namespace details {

enum class initation_point_type {
      USER_REQUEST
    , HEAP_GROWTH
    , AFTER_ALLOC
};

enum class gc_pause_type {
      GC
    , TRACE_ROOTS
    , SWEEP_HEAP
    , count
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

struct gc_state
{
    size_t              heap_size;
    size_t              heap_gain;
    gc_clock::duration  last_gc_time;
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

inline const char* pause_type_to_str(gc_pause_type pause_type)
{
    if (pause_type == gc_pause_type::GC) {
        return "full gc";
    } else if (pause_type == gc_pause_type::TRACE_ROOTS) {
        return "trace roots";
    } else if (pause_type == gc_pause_type::SWEEP_HEAP) {
        return "sweep heap";
    } else {
        return "undefined";
    }
}

inline std::string duration_to_str(gc_clock::duration duration)
{
    static const size_t ms = 1000;
    static const size_t  s = 1000 * ms;

    auto dur_us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

    char str[8];
    if (dur_us >= s) {
        snprintf(str, 8, "%4u s ", dur_us / s);
    } else if (dur_us >= ms) {
        snprintf(str, 8, "%4u ms", dur_us / ms);
    } else {
        snprintf(str, 8, "%4u us", dur_us);
    }
    return std::string(str);
}

inline std::string heapsize_to_str(size_t size)
{
    static const size_t Kb = 1024;
    static const size_t Mb = 1024 * Kb;
    static const size_t Gb = 1024 * Mb;

    char str[8];
    if (size >= Gb) {
        snprintf(str, 8, "%4u Gb", size / Gb);
    } else if (size >= Mb) {
        snprintf(str, 8, "%4u Mb", size / Mb);
    } else if (size >= Kb) {
        snprintf(str, 8, "%4u Kb", size / Kb);
    } else {
        snprintf(str, 8, "%4u b ", size);
    }
    return std::string(str);
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
