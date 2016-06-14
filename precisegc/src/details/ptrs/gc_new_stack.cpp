#include "libprecisegc/details/ptrs/gc_new_stack.hpp"

#include <cassert>
#include <cstdint>

namespace precisegc { namespace details { namespace ptrs {

gc_new_stack& gc_new_stack::instance()
{
    static thread_local gc_new_stack stack;
    return stack;
}

gc_new_stack::gc_new_stack()
    : m_top_ptr(nullptr)
    , m_top_size(0)
    , m_depth(0)
    , m_is_meta_requested(false)
    , m_top_offsets_cnt(0)
{}

void gc_new_stack::register_child(gc_untyped_ptr* child)
{
    std::uintptr_t top  = reinterpret_cast<std::uintptr_t>(m_top_ptr);
    std::uintptr_t curr = reinterpret_cast<std::uintptr_t>(child);
    if (top <= curr && curr < top + m_top_size) {
        push_offset(curr - top);
    }
}

gc_new_stack::offsets_range gc_new_stack::offsets() const
{
    return boost::make_iterator_range(m_top_offsets.begin(), std::next(m_top_offsets.begin(), m_top_offsets_cnt));
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
    return m_is_meta_requested;
}

void gc_new_stack::push_offset(size_t offset)
{
    if (m_top_offsets_cnt == m_top_offsets.size()) {
        utils::dynarray<size_t> new_offsets(2 * m_top_offsets_cnt);
        std::copy(m_top_offsets.begin(), m_top_offsets.end(), new_offsets.begin());
        m_top_offsets = new_offsets;
    }
    m_top_offsets[m_top_offsets_cnt++] = offset;
}

gc_new_stack::stack_entry::stack_entry(void* new_ptr, size_t new_size)
    : m_old_offsets(START_OFFSETS_STORAGE_SIZE)
{
    static thread_local gc_new_stack& stack = gc_new_stack::instance();
    assert(stack.is_active());

    m_old_ptr = stack.m_top_ptr;
    stack.m_top_ptr = new_ptr;

    m_old_size = stack.m_top_size;
    stack.m_top_size = new_size;

    m_old_is_meta_requested = stack.m_is_meta_requested;
    stack.m_is_meta_requested = true;

    m_old_offsets_cnt = stack.m_top_offsets_cnt;
    stack.m_top_offsets_cnt = 0;

    m_old_offsets.swap(stack.m_top_offsets);
}

gc_new_stack::stack_entry::~stack_entry()
{
    static thread_local gc_new_stack& stack = gc_new_stack::instance();
    stack.m_top_ptr = m_old_ptr;
    stack.m_top_size = m_old_size;
    stack.m_is_meta_requested = m_old_is_meta_requested;
    stack.m_top_offsets_cnt = m_old_offsets_cnt;
    stack.m_top_offsets.swap(m_old_offsets);
}

gc_new_stack::activation_entry::activation_entry()
{
    static thread_local gc_new_stack& stack = gc_new_stack::instance();
    stack.m_depth++;
}

gc_new_stack::activation_entry::~activation_entry()
{
    static thread_local gc_new_stack& stack = gc_new_stack::instance();
    stack.m_depth--;
}

}}}