#include "gc_untyped_ptr.h"

#include <cstdint>
#include <cassert>
#include <utility>
#include <details/logging.h>

#include "gc_new_stack.h"
#include "gc_unsafe_scope.h"
#include "../thread.h"

namespace precisegc { namespace details {

const uintptr_t gc_untyped_ptr::ROOT_FLAG_BIT = 1;


gc_untyped_ptr::gc_untyped_ptr() noexcept
    : gc_untyped_ptr(nullptr)
{}

//gc_untyped_ptr::gc_untyped_ptr(nullptr_t) noexcept
//    : gc_untyped_ptr()
//{}

gc_untyped_ptr::gc_untyped_ptr(void* ptr) noexcept
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
        register_root();
    }
}

gc_untyped_ptr::gc_untyped_ptr(const gc_untyped_ptr& other) noexcept
    : gc_untyped_ptr()
{
    gc_unsafe_scope unsafe_scope;
    set(other.m_ptr);
}

gc_untyped_ptr::gc_untyped_ptr(gc_untyped_ptr&& other) noexcept
    : gc_untyped_ptr()
{
    gc_unsafe_scope unsafe_scope;
    set(other.m_ptr);
}

gc_untyped_ptr::~gc_untyped_ptr() noexcept
{
    gc_unsafe_scope unsafe_scope;
    if (is_root()) {
        delete_root();
    }
}

gc_untyped_ptr& gc_untyped_ptr::operator=(nullptr_t t) noexcept
{
    set(nullptr);
    return *this;
}

gc_untyped_ptr& gc_untyped_ptr::operator=(const gc_untyped_ptr& other) noexcept
{
    gc_unsafe_scope unsafe_scope;
    set(other.m_ptr);
    return *this;
}

gc_untyped_ptr& gc_untyped_ptr::operator=(gc_untyped_ptr&& other) noexcept
{
    gc_unsafe_scope unsafe_scope;
    set(other.m_ptr);
    return *this;
}

void gc_untyped_ptr::swap(gc_untyped_ptr& other) noexcept
{
    gc_unsafe_scope unsafe_scope;
    void* tmp = m_ptr;
    m_ptr = set_root_flag(other.m_ptr, is_root());
    other.m_ptr = set_root_flag(tmp, other.is_root());
}

void* gc_untyped_ptr::get() const noexcept
{
    return clear_root_flag(m_ptr);
}

void gc_untyped_ptr::set(void* ptr) noexcept
{
    m_ptr = set_root_flag(ptr, is_root());
}

gc_untyped_ptr::operator bool() const noexcept
{
    return get() != nullptr;
}

bool gc_untyped_ptr::is_root() const noexcept
{
    return is_root_flag_set(m_ptr);
}

void gc_untyped_ptr::register_root() noexcept
{
    StackMap* root_set = get_thread_handler()->stack;
    root_set->register_stack_root(this);
}

void gc_untyped_ptr::delete_root() noexcept
{
    StackMap* root_set = get_thread_handler()->stack;
    root_set->delete_stack_root(this);
}

void* gc_untyped_ptr::set_root_flag(void* ptr, bool root_flag) noexcept
{
    uintptr_t uintptr = reinterpret_cast<uintptr_t>(ptr);
    if (root_flag) {
        return reinterpret_cast<void*>(uintptr | ROOT_FLAG_BIT);
    } else {
        return reinterpret_cast<void*>(uintptr & ~ROOT_FLAG_BIT);
    }
}

void* gc_untyped_ptr::clear_root_flag(void* ptr) noexcept
{
    return set_root_flag(ptr, false);
}

bool gc_untyped_ptr::is_root_flag_set(void* ptr) noexcept
{
    uintptr_t uintptr = reinterpret_cast<uintptr_t>(ptr);
    return uintptr & ROOT_FLAG_BIT;
}

void swap(gc_untyped_ptr& a, gc_untyped_ptr& b) noexcept
{
    a.swap(b);
}

}}