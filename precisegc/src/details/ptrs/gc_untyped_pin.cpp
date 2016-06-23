#include "libprecisegc/details/ptrs/gc_untyped_pin.hpp"

#include <libprecisegc/details/threads/managed_thread.hpp>

#include "gc_unsafe_scope.h"
#include "managed_ptr.hpp"

#include <libprecisegc/details/garbage_collector.hpp>

namespace precisegc { namespace details { namespace ptrs {

gc_untyped_pin::gc_untyped_pin(const gc_handle& handle)
    : m_ptr(gci().pin(handle))
{}

gc_untyped_pin::gc_untyped_pin(gc_untyped_pin&& other)
    : m_ptr(other.m_ptr)
{
    other.m_ptr = nullptr;
}

gc_untyped_pin::~gc_untyped_pin()
{
    gci().unpin((byte*) m_ptr);
}

gc_untyped_pin& gc_untyped_pin::operator=(gc_untyped_pin&& other)
{
    m_ptr = other.m_ptr;
    other.m_ptr = nullptr;
}

void* gc_untyped_pin::get() const noexcept
{
    return m_ptr;
}

}}}