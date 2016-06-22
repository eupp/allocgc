#include <libprecisegc/details/gc_handle.hpp>

#include <libprecisegc/details/garbage_collector.hpp>

namespace precisegc { namespace details {

gc_handle::gc_handle()
    : m_ptr(nullptr)
{}

gc_handle::gc_handle(byte* ptr)
    : m_ptr(ptr)
{}

byte* gc_handle::rbarrier() const
{
    return gci().rbarrier(*this);
}

void gc_handle::wbarrier(const gc_handle& other)
{
    gci().wbarrier(*this, other);
}

void gc_handle::interior_wbarrier(byte* ptr)
{
    gci().interior_wbarrier(*this, ptr);
}

void gc_handle::interior_shift(ptrdiff_t shift)
{
    gci().interior_shift(*this, shift);
}

void gc_handle::reset()
{
    gci().interior_wbarrier(*this, nullptr);
}

void gc_handle::pin() const
{
    gci().pin(*this);
}

void gc_handle::unpin() const
{
    gci().unpin(*this);
}

bool gc_handle::equal(const gc_handle& other) const
{
    return gci().compare(*this, other);
}

bool gc_handle::is_null() const
{
    return rbarrier() == nullptr;
}

byte* gc_handle::load(std::memory_order order) const
{
    return m_ptr.load(order);
}

void gc_handle::store(byte* ptr, std::memory_order order)
{
    m_ptr.store(ptr, order);
}

byte* gc_handle_access::load(const gc_handle& handle, std::memory_order order)
{
    return handle.load(order);
}

void gc_handle_access::store(gc_handle& handle, byte* ptr, std::memory_order order)
{
    handle.store(ptr, order);
}

}}