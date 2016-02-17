#include "gc_untyped_ptr.h"

#include <cstdint>
#include <cassert>
#include <utility>
#include <details/logging.h>

#include "gc_new_stack.h"
#include "gc_unsafe_scope.h"
#include "write_barrier.h"
#include "../thread.h"

namespace precisegc { namespace details {

gc_untyped_ptr::gc_untyped_ptr() noexcept
    : gc_untyped_ptr(nullptr)
{}

//gc_untyped_ptr::gc_untyped_ptr(nullptr_t) noexcept
//    : gc_untyped_ptr()
//{}

gc_untyped_ptr::gc_untyped_ptr(void* ptr) noexcept
    : m_ptr(ptr)
    , m_root_flag(gc_new_stack::instance().is_active())
{
    if (m_root_flag) {
        gc_new_stack& stack = gc_new_stack::instance();
        if (stack.is_meta_requsted()) {
            assert((void*) this >= stack.get_top_pointer());
            uintptr_t this_uintptr = reinterpret_cast<uintptr_t>(this);
            uintptr_t top_uintptr = reinterpret_cast<uintptr_t>(stack.get_top_pointer());
            stack.get_top_offsets().push_back(this_uintptr - top_uintptr);
        }
    } else {
        register_root();
    }
}

gc_untyped_ptr::gc_untyped_ptr(const gc_untyped_ptr& other) noexcept
    : gc_untyped_ptr()
{
    gc_unsafe_scope unsafe_scope;
    write_barrier(*this, other);
}

gc_untyped_ptr::gc_untyped_ptr(gc_untyped_ptr&& other) noexcept
    : gc_untyped_ptr()
{
    gc_unsafe_scope unsafe_scope;
    write_barrier(*this, other);
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
    write_barrier(*this, other);
    return *this;
}

gc_untyped_ptr& gc_untyped_ptr::operator=(gc_untyped_ptr&& other) noexcept
{
    gc_unsafe_scope unsafe_scope;
    write_barrier(*this, other);
    return *this;
}

void gc_untyped_ptr::swap(gc_untyped_ptr& other) noexcept
{
    gc_unsafe_scope unsafe_scope;
    gc_untyped_ptr tmp = (*this);
    (*this) = other;
    other = tmp;
}

void* gc_untyped_ptr::get() const noexcept
{
    return m_ptr.load();
}

void gc_untyped_ptr::set(void* ptr) noexcept
{
    m_ptr.store(ptr);
}

void gc_untyped_ptr::atomic_store(const gc_untyped_ptr& value)
{
    m_ptr.store(value.m_ptr);
}

gc_untyped_ptr::operator bool() const noexcept
{
    return get() != nullptr;
}

bool gc_untyped_ptr::is_root() const noexcept
{
    return m_root_flag;
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

void swap(gc_untyped_ptr& a, gc_untyped_ptr& b) noexcept
{
    a.swap(b);
}

}}