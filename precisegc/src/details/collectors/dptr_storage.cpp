#include <libprecisegc/details/collectors/dptr_storage.hpp>

namespace precisegc { namespace details { namespace collectors {

byte* dptr_storage::get(byte* ptr)
{
    return is_derived(ptr) ? get_derived_indirect(ptr) : ptr;
}

byte* dptr_storage::get_origin(byte* ptr)
{
    return is_derived(ptr) ? get_origin_indirect(ptr) : ptr;
}

bool dptr_storage::is_derived(byte* ptr)
{
    return reinterpret_cast<std::uintptr_t>(ptr) & DERIVED_BIT;
}

void dptr_storage::reset_derived_ptr(byte* ptr, ptrdiff_t offset)
{
    get_descriptor(ptr)->m_derived += offset;
}

void dptr_storage::forward_derived_ptr(byte* from, byte* to)
{
    dptr_descriptor* dscr = get_descriptor(from);
    ptrdiff_t offset = dscr->m_derived - dscr->m_origin;
    dscr->m_origin  = to;
    dscr->m_derived = to + offset;
}

dptr_storage::dptr_storage()
    : m_head(nullptr)
{ }

byte* dptr_storage::make_derived(byte* ptr, ptrdiff_t offset)
{
    dptr_descriptor* dscr = m_pool.create();
    dscr->m_origin  = ptr;
    dscr->m_derived = ptr + offset;
    dscr->m_next    = m_head;
    m_head = dscr;
    return set_derived_bit(reinterpret_cast<byte*>(dscr));
}

void dptr_storage::destroy_unmarked()
{
    while (m_head && !indexed_managed_object::get_mark(m_head->m_origin)) {
        dptr_descriptor* next = m_head->m_next;
        m_pool.destroy(m_head);
        m_head = next;
    }

    if (!m_head) {
        return;
    }

    dptr_descriptor* prev = m_head;
    dptr_descriptor* curr = m_head->m_next;
    while (curr) {
        if (!indexed_managed_object::get_mark(curr->m_origin)) {
            dptr_descriptor* next = curr->m_next;
            m_pool.destroy(curr);
            prev->m_next = next;
            curr = next;
        } else {
            prev = curr;
            curr = curr->m_next;
        }
    }
}

dptr_storage::dptr_descriptor * dptr_storage::get_descriptor(byte* ptr)
{
    return reinterpret_cast<dptr_descriptor*>(clear_derived_bit(ptr));
}

byte* dptr_storage::get_origin_indirect(byte* ptr)
{
    return get_descriptor(ptr)->m_origin;
}

byte* dptr_storage::get_derived_indirect(byte* ptr)
{
    return get_descriptor(ptr)->m_derived;
}

byte* dptr_storage::set_derived_bit(byte* ptr)
{
    return reinterpret_cast<byte*>(reinterpret_cast<std::uintptr_t>(ptr) | DERIVED_BIT);
}

byte * dptr_storage::clear_derived_bit(byte* ptr)
{
    return reinterpret_cast<byte*>(reinterpret_cast<std::uintptr_t>(ptr) & ~DERIVED_BIT);
}

}}}