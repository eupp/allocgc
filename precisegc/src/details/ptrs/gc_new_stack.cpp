#include "libprecisegc/details/ptrs/gc_new_stack.hpp"

#include <cassert>
#include <cstdint>

namespace precisegc { namespace details { namespace ptrs {

thread_local gc_new_stack::stack_top gc_new_stack::top{};

void gc_new_stack::stack_top::register_child(gc_untyped_ptr* child)
{
    assert(m_stack_top);
    std::uintptr_t top  = reinterpret_cast<std::uintptr_t>(m_stack_top->m_ptr);
    std::uintptr_t curr = reinterpret_cast<std::uintptr_t>(child);
    if (top <= curr && curr < top + m_stack_top->m_size) {
        m_stack_top->m_offsets.push_back(curr - top);
    }
}

gc_new_stack::offsets_range gc_new_stack::stack_top::offsets() const
{
    return boost::make_iterator_range(m_stack_top->m_offsets.begin(),
                                      m_stack_top->m_offsets.end());
}

size_t gc_new_stack::stack_top::depth() const noexcept
{
    return m_depth;
}

bool gc_new_stack::stack_top::is_active() const noexcept
{
    return m_depth > 0;
}

bool gc_new_stack::stack_top::is_meta_requsted() const noexcept
{
    return m_stack_top != nullptr;
}

gc_new_stack::stack_entry::stack_entry(void* ptr, size_t size)
    : m_ptr(ptr)
    , m_size(size)
    , m_prev(top.m_stack_top)
{
    assert(top.is_active());
    top.m_stack_top = this;
    m_offsets.reserve(START_OFFSETS_STORAGE_SIZE);
}

gc_new_stack::stack_entry::~stack_entry()
{
    top.m_stack_top = m_prev;
}

gc_new_stack::activation_entry::activation_entry()
{
    top.m_depth++;
}

gc_new_stack::activation_entry::~activation_entry()
{
    top.m_depth--;
}

void gc_new_stack::register_child(gc_untyped_ptr* child)
{
    top.register_child(child);
}

gc_new_stack::offsets_range gc_new_stack::offsets()
{
    return top.offsets();
}

size_t gc_new_stack::depth() noexcept
{
    return top.depth();
}

bool gc_new_stack::is_active() noexcept
{
    return top.is_active();
}

bool gc_new_stack::is_meta_requsted() noexcept
{
    return top.is_meta_requsted();
}

}}}