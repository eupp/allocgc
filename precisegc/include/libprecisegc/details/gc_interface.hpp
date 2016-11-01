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
    , GC_BAD_ALLOC
    , HEAP_EXPANSION
    , CONCURRENT_MARKING_FINISHED
    // for debugging
    , START_MARKING
    , START_COLLECTING
};

enum class gc_pause_type {
      GC
    , TRACE_ROOTS
    , SWEEP_HEAP
    , count
};

enum class gc_phase {
      IDLE
    , MARK
    , COLLECT
};

enum class gc_type {
      FULL_GC
    , TRACE_ROOTS
    , COLLECT_GARBAGE
    , SKIP_GC
};

struct gc_info
{
    bool    incremental_flag;
    bool    support_concurrent_marking;
    bool    support_concurrent_collecting;
};

struct gc_options
{
    gc_phase    phase;
};

struct gc_run_stats
{
    gc_type             type;
    size_t              mem_swept;
    size_t              mem_copied;
    gc_clock::duration  pause_duration;
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

    friend bool operator==(const incremental_gc_ops& a, const incremental_gc_ops& b)
    {
        return a.phase == b.phase && a.concurrent_flag == b.concurrent_flag && a.threads_num == b.threads_num;
    }

    friend bool operator!=(const incremental_gc_ops& a, const incremental_gc_ops& b)
    {
        return !(a == b);
    }
};

class gc_launcher
{
public:
    virtual gc_state state() const = 0;
    virtual void gc(gc_phase phase) = 0;
};

class initiation_point_data
{
    struct heap_expansion_data
    {
        size_t alloc_size;
    };

    struct empty_data {};

    typedef boost::variant<heap_expansion_data, empty_data> variant_t;
public:
    initiation_point_data(const initiation_point_data&) = default;
    initiation_point_data(initiation_point_data&&) = default;

    initiation_point_data& operator=(const initiation_point_data&) = default;
    initiation_point_data& operator=(initiation_point_data&&) = default;

    static initiation_point_data create_heap_expansion_data(size_t alloc_size)
    {
        heap_expansion_data data = { alloc_size };
        return initiation_point_data(data);
    }

    static initiation_point_data create_empty_data()
    {
        return initiation_point_data(empty_data());
    }

    size_t alloc_size() const
    {
        const heap_expansion_data* data = boost::get<heap_expansion_data>(&m_data);
        assert(data);
        return data->alloc_size;
    }
private:
    initiation_point_data(const variant_t& data)
        : m_data(data)
    {}

    variant_t m_data;
};

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

inline const char* gc_type_to_str(gc_type type)
{
    if (type == gc_type::FULL_GC) {
        return "full gc";
    } else if (type == gc_type::TRACE_ROOTS) {
        return "trace roots";
    } else if (type == gc_type::COLLECT_GARBAGE) {
        return "collect garbage";
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
