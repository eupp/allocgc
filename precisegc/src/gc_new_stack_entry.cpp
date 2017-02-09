#include <libprecisegc/gc_new_stack_entry.hpp>

#include <libprecisegc/details/gc_hooks.hpp>

namespace precisegc {

gc_new_stack_entry::gc_new_stack_entry(byte* ptr, size_t size, bool meta_requested)
    : m_ptr(ptr)
    , m_size(size)
    , m_meta_requested(meta_requested)
{
    details::gc_register_stack_entry(this);
}

gc_new_stack_entry::~gc_new_stack_entry()
{
    details::gc_deregister_stack_entry(this);
}

byte* gc_new_stack_entry::get_ptr() const
{
    return m_ptr;
}

gc_new_stack_entry* gc_new_stack_entry::get_prev() const
{
    return m_prev;
}

void gc_new_stack_entry::set_prev(gc_new_stack_entry* prev)
{
    m_prev = prev;
}

void gc_new_stack_entry::register_offset(size_t offset)
{
    m_offsets.push_back(offset);
}

bool gc_new_stack_entry::is_meta_requested() const
{
    return m_meta_requested;
}

gc_new_stack_entry::offsets_range gc_new_stack_entry::offsets() const
{
    return boost::make_iterator_range(m_offsets.begin(), m_offsets.end());
}

}
