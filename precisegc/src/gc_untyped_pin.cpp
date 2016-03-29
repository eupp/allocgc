#include "gc_untyped_pin.h"

#include "gc_unsafe_scope.h"
#include "managed_ptr.h"

namespace precisegc { namespace details {

gc_untyped_pin::gc_untyped_pin(const gc_untyped_ptr& ptr)
{
    gc_unsafe_scope unsafe_scope;
    m_raw_ptr = ptr.get();
    m_descr = managed_ptr(reinterpret_cast<byte*>(m_raw_ptr)).get_indexed_entry();
    m_descr->set_pin(reinterpret_cast<byte*>(m_raw_ptr), true);
}

gc_untyped_pin::~gc_untyped_pin()
{
    gc_unsafe_scope unsafe_scope;
    m_descr->set_pin(reinterpret_cast<byte*>(m_raw_ptr), false);
}

void* gc_untyped_pin::get() const noexcept
{
    return m_raw_ptr;
}

}}