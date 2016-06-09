#include "libprecisegc/details/ptrs/gc_untyped_pin.hpp"

#include <libprecisegc/details/threads/managed_thread.hpp>

#include "gc_unsafe_scope.h"
#include "managed_ptr.hpp"

namespace precisegc { namespace details { namespace ptrs {

gc_untyped_pin::gc_untyped_pin(const gc_untyped_ptr& ptr)
{
    gc_unsafe_scope unsafe_scope;
    m_ptr = ptr.get();
    static thread_local pin_stack_map& pin_set = threads::managed_thread::this_thread().pin_set();
    pin_set.insert((byte*) m_ptr);
}

gc_untyped_pin::~gc_untyped_pin()
{
    // here we not use unsafe_scope because it is always used in delete_stack_root
    // and taking pinned raw pointer is safe
    static thread_local pin_stack_map& pin_set = threads::managed_thread::this_thread().pin_set();
    pin_set.remove((byte*) m_ptr);
}

void* gc_untyped_pin::get() const noexcept
{
    return m_ptr;
}

}}}