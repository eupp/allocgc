#include "gc_new_state.h"

#include <cassert>

namespace precisegc { namespace details {

thread_local std::unique_ptr<gc_new_state> gc_new_state::state_ptr = nullptr;

gc_new_state& gc_new_state::instance()
{
    if (!state_ptr) {
        state_ptr.reset(new gc_new_state);
    }
    return * state_ptr;
}

gc_new_state::gc_new_state()
    : m_current_ptr(nullptr)
    , m_is_active(false)
{}

std::vector <size_t>& gc_new_state::get_offsets()
{
    return m_offsets;
}

void* gc_new_state::get_current_pointer() const noexcept
{
    return m_current_ptr;
}

void gc_new_state::set_current_pointer(void* ptr) noexcept
{
    m_current_ptr = ptr;
}

bool gc_new_state::is_active() const noexcept
{
    return m_is_active;
}

void gc_new_state::set_active(bool active) noexcept
{
    m_is_active = active;
}

gc_new_state::stack_entry::stack_entry()
    : m_old_current_ptr(nullptr)
    , m_new_state_set(false)
{
    gc_new_state& state = gc_new_state::instance();
    m_old_is_active = state.is_active();
    state.set_active(true);
}

gc_new_state::stack_entry::~stack_entry()
{
    gc_new_state& state = gc_new_state::instance();
    state.set_active(m_old_is_active);
    if (m_new_state_set) {
        state.set_current_pointer(m_old_current_ptr);
        state.get_offsets().swap(m_old_offsets);
    }
}

void gc_new_state::stack_entry::push_new_state(void* new_ptr)
{
    assert(!m_new_state_set);
    gc_new_state& state = gc_new_state::instance();
    m_old_current_ptr = state.get_current_pointer();
    state.set_current_pointer(new_ptr);
    m_old_offsets.swap(state.get_offsets());
    m_new_state_set = true;
}

std::vector<size_t>& gc_new_state::stack_entry::get_offsets()
{
    return gc_new_state::instance().get_offsets();
}

}}