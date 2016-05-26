#include <libprecisegc/details/gc_hooks.hpp>

#include <cassert>
#include <memory>

#include <libprecisegc/details/gc_interface.hpp>

namespace precisegc { namespace details {

static std::unique_ptr<gc_interface> gc = nullptr;

void gc_init(gc_options opts)
{

}

managed_ptr gc_allocate(size_t size)
{
    assert(gc);
    return gc->allocate(size);
}

void gc_write_barrier(gc_untyped_ptr& dst, const gc_untyped_ptr& src)
{
    assert(gc);
    gc->write_barrier(dst, src);
}

byte* gc_load(const atomic_byte_ptr& p)
{
    assert(gc);
    return gc->load_ptr(p);
}

void gc_store(atomic_byte_ptr& p, byte* value)
{
    assert(gc);
    gc->store_ptr(p, value);
}

}}
