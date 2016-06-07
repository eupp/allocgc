#ifndef DIPLOMA_GC_STRATEGY_HPP
#define DIPLOMA_GC_STRATEGY_HPP

#include <chrono>

#include <libprecisegc/gc_options.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/managed_ptr.hpp>
#include <libprecisegc/details/gc_exception.hpp>

namespace precisegc { namespace details {

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

#endif //DIPLOMA_GC_STRATEGY_HPP
