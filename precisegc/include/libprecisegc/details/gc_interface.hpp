#ifndef DIPLOMA_GC_INTERFACE_HPP
#define DIPLOMA_GC_INTERFACE_HPP

#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_exception.hpp>

namespace precisegc { namespace details {

enum class initation_point_type {
    AFTER_ALLOC
};

enum class gc_phase {
      IDLING
    , MARKING
    , SWEEPING
};

struct gc_stat
{
    size_t  heap_size;
    bool    incremental;
    bool    support_concurrent_mark;
    bool    support_concurrent_sweep;
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

class gc_interface
{
public:
    virtual ~gc_interface() {}

    virtual managed_ptr allocate(size_t size) = 0;

    virtual byte* rbarrier(const atomic_byte_ptr& p) = 0;
    virtual void  wbarrier(atomic_byte_ptr& dst, const atomic_byte_ptr& src) = 0;

    virtual void initation_point(initation_point_type ipoint) = 0;

    virtual gc_stat stat() const = 0;
};

class serial_gc_interface : public gc_interface
{
public:
    virtual void gc() = 0;
};

class incremental_gc_interface : public serial_gc_interface
{
public:
    virtual gc_phase phase() const = 0;

    virtual void incremental_gc(const incremental_gc_ops& ops) = 0;
};

}}

#endif //DIPLOMA_GC_INTERFACE_HPP
