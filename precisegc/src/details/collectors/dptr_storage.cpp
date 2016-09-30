#include <libprecisegc/details/collectors/dptr_storage.hpp>

namespace precisegc { namespace details { namespace collectors {

byte* dptr_storage::get(const gc_handle* handle)
{
    byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(*handle);
    return derived_bit(ptr) ? get_derived(ptr) : ptr;
}

byte* dptr_storage::get_origin(const gc_handle* handle)
{
    byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(*handle);
    return derived_bit(ptr) ? get_origin(ptr) : ptr;
}

byte* dptr_storage::make_derived(const gc_handle* handle, ptrdiff_t offset)
{
    byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(*handle);
    dptr_descriptor* dscr = m_pool.create();
    dscr->m_origin  = ptr;
    dscr->m_derived = ptr + offset;
    return reinterpret_cast<byte*>(dscr);
}

void dptr_storage::destroy_unmarked()
{

}

byte * dptr_storage::get_origin(byte* ptr)
{
    return reinterpret_cast<dptr_descriptor*>(clear_derived_bit(ptr))->m_origin;
}

byte* dptr_storage::get_derived(byte* ptr)
{
    return reinterpret_cast<dptr_descriptor*>(clear_derived_bit(ptr))->m_derived;
}

bool dptr_storage::derived_bit(byte* ptr)
{
    return reinterpret_cast<std::uintptr_t>(ptr) & DERIVED_BIT;
}

void dptr_storage::set_derived_bit(byte*& ptr)
{
    ptr = reinterpret_cast<byte*>(reinterpret_cast<std::uintptr_t>(ptr) | DERIVED_BIT);
}

byte * dptr_storage::clear_derived_bit(byte* ptr)
{
    return reinterpret_cast<byte*>(reinterpret_cast<std::uintptr_t>(ptr) & ~DERIVED_BIT);
}

}}}