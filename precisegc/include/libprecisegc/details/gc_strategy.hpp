#ifndef DIPLOMA_GC_STRATEGY_HPP
#define DIPLOMA_GC_STRATEGY_HPP

#include <chrono>

#include <libprecisegc/gc_options.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/managed_ptr.hpp>
#include <libprecisegc/details/gc_handle.hpp>

namespace precisegc { namespace details {

class gc_strategy
{
public:
    virtual ~gc_strategy() {}

    virtual managed_ptr allocate(size_t size) = 0;

    virtual byte* rbarrier(const gc_handle& p) = 0;
    virtual void  wbarrier(gc_handle& dst, const gc_handle& src) = 0;

    virtual void interior_wbarrier(gc_handle& handle, byte* ptr) = 0;
    virtual void interior_shift(gc_handle& handle, ptrdiff_t shift) = 0;

    virtual byte* pin(const gc_handle& handle) = 0;
    virtual void  unpin(byte* ptr) = 0;

    virtual bool compare(const gc_handle& a, const gc_handle& b) = 0;

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

#endif //DIPLOMA_GC_STRATEGY_HPP
