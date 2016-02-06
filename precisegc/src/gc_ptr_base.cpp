#include "gc_ptr_base.h"

#include <cstdint>
#include <cassert>
#include <utility>

#include "gc_new_stack.h"
#include "../thread.h"

namespace precisegc { namespace details {

const uintptr_t gc_ptr_base::ROOT_FLAG_BIT = 1;


gc_ptr_base::gc_ptr_base() noexcept
    : gc_ptr_base(nullptr)
{}

gc_ptr_base::gc_ptr_base(void* ptr) noexcept
{
    gc_new_stack& stack = gc_new_stack::instance();
    if (stack.is_active()) {
        m_ptr = set_root_flag(ptr, false);
        if (stack.is_meta_requsted()) {
            assert((void*) this >= stack.get_top_pointer());
            uintptr_t this_uintptr = reinterpret_cast<uintptr_t>(this);
            uintptr_t top_uintptr = reinterpret_cast<uintptr_t>(stack.get_top_pointer());
            stack.get_top_offsets().push_back(this_uintptr - top_uintptr);
        }
    } else {
        m_ptr = set_root_flag(ptr, true);
        StackMap* root_set = get_thread_handler()->stack;
        root_set->register_stack_root(this);
    }
}

gc_ptr_base::gc_ptr_base(const gc_ptr_base& other) noexcept
    : gc_ptr_base(other.m_ptr)
{}

gc_ptr_base::gc_ptr_base(gc_ptr_base&& other) noexcept
    : gc_ptr_base(other.m_ptr)
{}

gc_ptr_base::~gc_ptr_base() noexcept
{
    if (is_root()) {
        StackMap* root_set = get_thread_handler()->stack;
        root_set->delete_stack_root(this);
    }
}

gc_ptr_base& gc_ptr_base::operator=(nullptr_t t) noexcept
{
    set(nullptr);
    return *this;
}

gc_ptr_base& gc_ptr_base::operator=(const gc_ptr_base& other) noexcept
{
    set(other.m_ptr);
    return *this;
}

gc_ptr_base& gc_ptr_base::operator=(gc_ptr_base&& other) noexcept
{
    set(other.m_ptr);
    return *this;
}

void gc_ptr_base::swap(gc_ptr_base& other) noexcept
{
    void* tmp = m_ptr;
    m_ptr = set_root_flag(other.m_ptr, is_root());
    other.m_ptr = set_root_flag(tmp, other.is_root());
}

void* gc_ptr_base::get() const noexcept
{
    return clear_root_flag(m_ptr);
}

void gc_ptr_base::set(void* ptr) noexcept
{
    m_ptr = set_root_flag(ptr, is_root());
}

gc_ptr_base::operator bool() const noexcept
{
    return get() != nullptr;
}

bool gc_ptr_base::is_root() const noexcept
{
    return is_root_flag_set(m_ptr);
}

void* gc_ptr_base::set_root_flag(void* ptr, bool root_flag) noexcept
{
    uintptr_t uintptr = reinterpret_cast<uintptr_t>(ptr);
    if (root_flag) {
        return reinterpret_cast<void*>(uintptr | ROOT_FLAG_BIT);
    } else {
        return reinterpret_cast<void*>(uintptr & ~ROOT_FLAG_BIT);
    }
}

void* gc_ptr_base::clear_root_flag(void* ptr) noexcept
{
    return set_root_flag(ptr, false);
}

bool gc_ptr_base::is_root_flag_set(void* ptr) noexcept
{
    uintptr_t uintptr = reinterpret_cast<uintptr_t>(ptr);
    return uintptr & ROOT_FLAG_BIT;
}

void swap(gc_ptr_base& a, gc_ptr_base& b) noexcept
{
    a.swap(b);
}

}}