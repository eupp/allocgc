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

byte* gc_rbarrier(const atomic_byte_ptr& p)
{
    assert(gc);
    return gc->rbarrier(p);
}

void gc_wbarrier(atomic_byte_ptr& dst, const atomic_byte_ptr& src)
{
    assert(gc);
    gc->wbarrier(dst, src);
}

}}
