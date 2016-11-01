#ifndef DIPLOMA_GC_INTERFACE_HPP
#define DIPLOMA_GC_INTERFACE_HPP

#include <string>

#include <boost/variant.hpp>

#include <libprecisegc/details/utils/block_ptr.hpp>
#include <libprecisegc/details/collectors/indexed_managed_object.hpp>
#include <libprecisegc/details/collectors/traceable_object_meta.hpp>
#include <libprecisegc/details/gc_exception.hpp>
#include <libprecisegc/details/gc_clock.hpp>

namespace precisegc { namespace details {

enum class initiation_point_type {
      USER_REQUEST
    , HEAP_LIMIT_EXCEEDED
    , CONCURRENT_MARKING_FINISHED
    // for debugging
    , START_MARKING
    , START_COLLECTING
};

enum class gc_pause_type {
      MARK_COLLECT
    , TRACE_ROOTS
    , COLLECT
    , SKIP
    , count
};

enum class gc_phase {
      IDLE
    , MARK
    , COLLECT
};

enum class gc_kind {
      MARK_COLLECT
    , CONCURRENT_MARK
    , COLLECT
    , SKIP
};

typedef int     gc_gen;

struct gc_info
{
    bool    incremental_flag;
    bool    support_concurrent_marking;
    bool    support_concurrent_collecting;
};

struct gc_options
{
    gc_kind     kind;
    gc_gen      gen;
};

struct gc_pause_stat
{
    gc_pause_type       type      = gc_pause_type::SKIP;
    gc_clock::duration  duration  = gc_clock::duration(0);
};

struct gc_heap_stat
{
    gc_heap_stat() = default;

    gc_heap_stat(const gc_heap_stat&) = default;

    gc_heap_stat& operator=(const gc_heap_stat&) = default;

    gc_heap_stat& operator+=(const gc_heap_stat& other)
    {
        mem_all         += other.mem_all;
        mem_live        += other.mem_live;
        mem_freed       += other.mem_freed;
        mem_copied      += other.mem_copied;
        pinned_cnt      += other.pinned_cnt;

        return *this;
    }

    double residency() const
    {
        return mem_live / mem_all;
    }

    size_t mem_before_gc = 0;
    size_t mem_all       = 0;
    size_t mem_live      = 0;
    size_t mem_freed     = 0;
    size_t mem_copied    = 0;
    size_t pinned_cnt    = 0;
};

inline gc_heap_stat operator+(const gc_heap_stat& a, const gc_heap_stat& b)
{
    return gc_heap_stat(a) += b;
}

struct gc_run_stats
{
    gc_heap_stat    heap_stat;
    gc_pause_stat   pause_stat;
};

class gc_launcher
{
public:
    virtual void gc(const gc_options& opt) = 0;
};

inline const char* gc_pause_type_to_str(gc_pause_type pause_type)
{
    if (pause_type == gc_pause_type::MARK_COLLECT) {
        return "full gc";
    } else if (pause_type == gc_pause_type::TRACE_ROOTS) {
        return "trace roots";
    } else if (pause_type == gc_pause_type::COLLECT) {
        return "collect";
    } else {
        return "undefined";
    }
}

inline const char* gc_kind_to_str(gc_kind type)
{
    if (type == gc_kind::MARK_COLLECT) {
        return "mark collect";
    } else if (type == gc_kind::CONCURRENT_MARK) {
        return "concurrent mark";
    } else if (type == gc_kind::COLLECT) {
        return "collect";
    } else {
        return "undefined";
    }
}

inline std::string duration_to_str(gc_clock::duration duration, int padding = 0)
{
    static const size_t ms = 1000;
    static const size_t  s = 1000 * ms;

    size_t dur_us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

    char str[8];
    if (dur_us >= s) {
        snprintf(str, 8, "%*lu s ", padding, dur_us / s);
    } else if (dur_us >= ms) {
        snprintf(str, 8, "%*lu ms", padding, dur_us / ms);
    } else {
        snprintf(str, 8, "%*lu us", padding, dur_us);
    }
    return std::string(str);
}

inline std::string heapsize_to_str(size_t size, int padding = 0)
{
    static const size_t Kb = 1024;
    static const size_t Mb = 1024 * Kb;
    static const size_t Gb = 1024 * Mb;

    char str[8];
    if (size >= Gb) {
        snprintf(str, 8, "%*lu Gb", padding, size / Gb);
    } else if (size >= Mb) {
        snprintf(str, 8, "%*lu Mb", padding, size / Mb);
    } else if (size >= Kb) {
        snprintf(str, 8, "%*lu Kb", padding, size / Kb);
    } else {
        snprintf(str, 8, "%*lu b ", padding, size);
    }
    return std::string(str);
}

}}

#endif //DIPLOMA_GC_INTERFACE_HPP
