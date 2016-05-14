#include "gc_untyped_pin.h"

#include <libprecisegc/details/threads/managed_thread.hpp>

#include "gc_unsafe_scope.h"
#include "managed_ptr.h"

namespace precisegc { namespace details {

gc_untyped_pin::gc_untyped_pin(const gc_untyped_ptr& ptr)
{
    gc_unsafe_scope unsafe_scope;
    m_ptr = &ptr;
    static thread_local root_set& pin_set = threads::managed_thread::this_thread().get_pin_set();
    pin_set.add((gc_untyped_ptr*) m_ptr);
}

gc_untyped_pin::~gc_untyped_pin()
{
    // here we not use unsafe_scope because it is always used in delete_stack_root
    // and taking pinned raw pointer is safe
    gc_unsafe_scope unsafe_scope;
    static thread_local root_set& pin_set = threads::managed_thread::this_thread().get_pin_set();
    pin_set.remove((gc_untyped_ptr*) m_ptr);
}

void* gc_untyped_pin::get() const noexcept
{
    return m_ptr->get();
}

}}