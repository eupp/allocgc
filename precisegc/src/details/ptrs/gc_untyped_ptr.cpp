#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>

#include <cstdint>
#include <cassert>
#include <utility>

#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/ptrs/gc_new_stack.hpp>
#include <libprecisegc/details/garbage_collector.hpp>
#include <libprecisegc/details/logging.h>

namespace precisegc { namespace details { namespace ptrs {

gc_untyped_ptr::gc_untyped_ptr()
    : gc_untyped_ptr(nullptr)
{}

//gc_untyped_ptr::gc_untyped_ptr(nullptr_t) noexcept
//    : gc_untyped_ptr()
//{}

gc_untyped_ptr::gc_untyped_ptr(void* ptr)
    : m_handle(reinterpret_cast<byte*>(ptr))
    , m_root_flag(!gc_new_stack::is_active())
{
    if (m_root_flag) {
        register_root();
    } else {
        if (gc_new_stack::is_meta_requsted()) {
            gc_new_stack::register_child(this);
        }
    }
}

gc_untyped_ptr::gc_untyped_ptr(const gc_untyped_ptr& other)
    : gc_untyped_ptr()
{
    m_handle.wbarrier(other.m_handle);
}

gc_untyped_ptr::gc_untyped_ptr(gc_untyped_ptr&& other)
    : gc_untyped_ptr()
{
    m_handle.wbarrier(other.m_handle);
}

gc_untyped_ptr::~gc_untyped_ptr()
{
    if (is_root()) {
        delete_root();
    }
}

gc_untyped_ptr& gc_untyped_ptr::operator=(nullptr_t t)
{
    m_handle.reset();
    return *this;
}

gc_untyped_ptr& gc_untyped_ptr::operator=(const gc_untyped_ptr& other)
{
    m_handle.wbarrier(other.m_handle);
    return *this;
}

gc_untyped_ptr& gc_untyped_ptr::operator=(gc_untyped_ptr&& other)
{
    m_handle.wbarrier(other.m_handle);
    return *this;
}

gc_untyped_pin gc_untyped_ptr::untyped_pin() const
{
    return gc_untyped_pin(m_handle);
}

bool gc_untyped_ptr::is_null() const
{
    return m_handle.is_null();
}

bool gc_untyped_ptr::is_root() const
{
    return m_root_flag;
}

bool gc_untyped_ptr::equal(const gc_untyped_ptr& other) const
{
    return m_handle.equal(other.m_handle);
}

void gc_untyped_ptr::advance(ptrdiff_t n)
{
    m_handle.interior_shift(n);
}

void* gc_untyped_ptr::get() const
{
    return m_handle.rbarrier();
}

void gc_untyped_ptr::forward(void* ptr)
{
    gc_handle_access::store(m_handle, (byte*) ptr, std::memory_order_relaxed);
}

void gc_untyped_ptr::swap(gc_untyped_ptr& other)
{
    gc_untyped_ptr tmp = (*this);
    (*this) = other;
    other = tmp;
}

void swap(gc_untyped_ptr& a, gc_untyped_ptr& b)
{
    a.swap(b);
}

void gc_untyped_ptr::register_root()
{
    static thread_local root_stack_map& root_set = threads::managed_thread::this_thread().root_set();
    root_set.insert(this);
}

void gc_untyped_ptr::delete_root()
{
    static thread_local root_stack_map& root_set = threads::managed_thread::this_thread().root_set();
    root_set.remove(this);
}

}}}