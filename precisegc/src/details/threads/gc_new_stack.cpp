#include <libprecisegc/details/threads/gc_new_stack.hpp>

#include <cassert>
#include <cstdint>

#include <libprecisegc/details/threads/this_managed_thread.hpp>

namespace precisegc { namespace details { namespace threads {

gc_new_stack::gc_new_stack()
    : m_stack_top(nullptr)
    , m_depth(0)
{}

void gc_new_stack::push_entry(gc_new_stack::stack_entry* entry)
{
    entry->m_prev = m_stack_top;
    m_stack_top = entry;
    ++m_depth;
}

void gc_new_stack::pop_entry()
{
    m_stack_top = m_stack_top->m_prev;
    --m_depth;
}

void gc_new_stack::register_child(byte* child) const
{
    assert(m_stack_top);
    std::uintptr_t top  = reinterpret_cast<std::uintptr_t>(m_stack_top->m_ptr);
    std::uintptr_t curr = reinterpret_cast<std::uintptr_t>(child);
    m_stack_top->m_offsets.push_back(curr - top);
}

gc_new_stack::offsets_range gc_new_stack::offsets() const
{
    return boost::make_iterator_range(m_stack_top->m_offsets.begin(),
                                      m_stack_top->m_offsets.end());
}

size_t gc_new_stack::depth() const noexcept
{
    return m_depth;
}

bool gc_new_stack::is_active() const noexcept
{
    return m_depth > 0;
}

bool gc_new_stack::is_meta_requsted() const noexcept
{
//    assert(m_stack_top);
    return m_stack_top && m_stack_top->m_meta_requested;
}

//bool gc_new_stack::is_heap_ptr(byte* ptr) const noexcept
//{
//    return m_stack_top && (m_stack_top->m_ptr <= ptr) && (ptr < m_stack_top->m_ptr + m_stack_top->m_size);
//}

gc_new_stack::stack_entry::stack_entry(byte* ptr, size_t size, bool meta_requested)
    : m_ptr(ptr)
    , m_size(size)
    , m_meta_requested(meta_requested)
{
    this_managed_thread::push_on_gc_new_stack(this);
}

gc_new_stack::stack_entry::~stack_entry()
{
    this_managed_thread::pop_from_gc_new_stack();
}

}}}