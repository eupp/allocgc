#include "gc_new_stack.h"

#include <cassert>

namespace precisegc { namespace details {

thread_local std::unique_ptr<gc_new_stack> gc_new_stack::state_ptr = nullptr;

gc_new_stack& gc_new_stack::instance()
{
    if (!state_ptr) {
        state_ptr.reset(new gc_new_stack);
    }
    return * state_ptr;
}

gc_new_stack::gc_new_stack()
    : m_top_ptr(nullptr)
    , m_nesting_level(0)
    , m_is_meta_requested(false)
{}

std::vector<size_t>& gc_new_stack::get_top_offsets()
{
    return m_top_offsets;
}

void* gc_new_stack::get_top_pointer() const noexcept
{
    return m_top_ptr;
}

bool gc_new_stack::is_active() const noexcept
{
    return m_nesting_level > 0;
}

bool gc_new_stack::is_meta_requsted() const noexcept
{
    return m_is_meta_requested;
}

gc_new_stack::stack_entry::stack_entry(void* new_ptr)
{
    gc_new_stack& stack = gc_new_stack::instance();
    assert(stack.is_active());
    m_old_ptr = stack.m_top_ptr;
    stack.m_top_ptr = new_ptr;
    m_old_is_meta_requested = stack.m_is_meta_requested;
    stack.m_is_meta_requested = true;
    m_old_offsets.swap(stack.m_top_offsets);
}

gc_new_stack::stack_entry::~stack_entry()
{
    gc_new_stack& stack = gc_new_stack::instance();
    stack.m_top_ptr = m_old_ptr;
    stack.m_is_meta_requested = m_old_is_meta_requested;
    stack.m_top_offsets.swap(m_old_offsets);
}

gc_new_stack::activation_entry::activation_entry()
{
    gc_new_stack& stack = gc_new_stack::instance();
    stack.m_nesting_level++;
}

gc_new_stack::activation_entry::~activation_entry()
{
    gc_new_stack& stack = gc_new_stack::instance();
    stack.m_nesting_level--;
}

}}