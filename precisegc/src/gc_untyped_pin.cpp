#include "gc_untyped_pin.h"

#include "gc_unsafe_scope.h"
#include "managed_ptr.h"

#include "logging.h"

namespace precisegc { namespace details {

gc_untyped_pin::gc_untyped_pin(const gc_untyped_ptr& ptr)
{
    gc_unsafe_scope unsafe_scope;
    m_raw_ptr = ptr.get();
    static thread_local StackMap* pin_set = get_thread_handler()->pins.get();
    pin_set->register_stack_root(m_raw_ptr);
}

gc_untyped_pin::~gc_untyped_pin()
{
    gc_unsafe_scope unsafe_scope;
    static thread_local StackMap* pin_set = get_thread_handler()->pins.get();
    pin_set->delete_stack_root(m_raw_ptr);
}

void* gc_untyped_pin::get() const noexcept
{
    return m_raw_ptr;
}

}}