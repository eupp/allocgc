#ifndef DIPLOMA_GC_INTERFACE_HPP
#define DIPLOMA_GC_INTERFACE_HPP

#include <libprecisegc/details/gc_hooks.hpp>

namespace precisegc { namespace details {

class gc_interface
{
public:
    virtual void gc() = 0;

    virtual managed_ptr allocate(size_t size) = 0;

    virtual void write_barrier(gc_untyped_ptr& dst, const gc_untyped_ptr& src) = 0;

    virtual byte* load_ptr(const atomic_byte_ptr& p) const = 0;
    virtual void  store_ptr(atomic_byte_ptr& p, byte* value) const = 0;
};

class gc_incremental_interface : public gc_interface
{
public:
    virtual void start_marking() = 0;
};

}}

#endif //DIPLOMA_GC_INTERFACE_HPP
