#ifndef DIPLOMA_GC_INTERFACE_HPP
#define DIPLOMA_GC_INTERFACE_HPP

#include <chrono>

#include <libprecisegc/gc_options.hpp>
#include <libprecisegc/details/managed_ptr.hpp>
#include <libprecisegc/details/gc_exception.hpp>

namespace precisegc { namespace details {

enum class initation_point_type {
      USER_REQUEST
    , AFTER_ALLOC
};

enum class gc_pause_type {
      TRACE_ROOTS
    , SWEEP_HEAP
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

typedef std::chrono::steady_clock::time_point gc_time_point;
typedef std::chrono::steady_clock::duration gc_duration;

struct gc_stat
{
    size_t          heap_size;
    size_t          heap_gain;
    gc_duration     last_alloc_timediff;
    gc_duration     last_gc_timediff;
    gc_duration     last_pause_time;
};

struct gc_pause_stat
{
    gc_pause_type   type;
    gc_duration     duration;
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

class gc_strategy
{
public:
    virtual ~gc_strategy() {}

    virtual managed_ptr allocate(size_t size) = 0;

    virtual byte* rbarrier(const atomic_byte_ptr& p) = 0;
    virtual void  wbarrier(atomic_byte_ptr& dst, const atomic_byte_ptr& src) = 0;

    virtual void initation_point(initation_point_type ipoint) = 0;

    virtual gc_info info() const = 0;
};

class serial_gc_strategy : public gc_strategy
{
public:
    virtual void gc() = 0;
};

class incremental_gc_strategy : public serial_gc_strategy
{
public:
    virtual gc_phase phase() const = 0;

    virtual void gc_increment(const incremental_gc_ops& ops) = 0;
};

}}

#endif //DIPLOMA_GC_INTERFACE_HPP
