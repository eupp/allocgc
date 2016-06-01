#include <libprecisegc/details/gc_hooks.hpp>

#include <cassert>
#include <memory>

namespace precisegc { namespace details {

static std::unique_ptr<gc_interface> garbage_collector = nullptr;

gc_interface* gc_get()
{
    return garbage_collector.get();
}

void gc_set(std::unique_ptr<gc_interface> gc)
{
    garbage_collector = std::move(gc);
}

std::unique_ptr<gc_interface> gc_reset(std::unique_ptr<gc_interface> gc)
{
    std::unique_ptr<gc_interface> old = std::move(garbage_collector);
    garbage_collector = std::move(gc);
    return old;
}

void gc()
{
    assert(garbage_collector);
    garbage_collector->initation_point(initation_point_type::USER_REQUEST);
};

managed_ptr gc_allocate(size_t size)
{
    assert(garbage_collector);
    return garbage_collector->allocate(size);
}

byte* gc_rbarrier(const atomic_byte_ptr& p)
{
    assert(garbage_collector);
    return garbage_collector->rbarrier(p);
}

void gc_wbarrier(atomic_byte_ptr& dst, const atomic_byte_ptr& src)
{
    assert(garbage_collector);
    garbage_collector->wbarrier(dst, src);
}

}}
