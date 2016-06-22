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
    // here we not use unsafe_scope because it is always used in delete_stack_root
    // and taking pinned raw pointer is safe
    if (m_ptr) {
        static thread_local pin_stack_map& pin_set = threads::managed_thread::this_thread().pin_set();
        pin_set.remove((byte*) m_ptr);
    }
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