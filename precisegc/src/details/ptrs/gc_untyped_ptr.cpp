#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>

#include <cstdint>
#include <cassert>
#include <utility>

#include <libprecisegc/details/threads/this_managed_thread.hpp>
#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/gc_tagging.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace ptrs {

gc_untyped_ptr::gc_untyped_ptr()
    : gc_untyped_ptr(nullptr)
{}

//gc_untyped_ptr::gc_untyped_ptr(nullptr_t) noexcept
//    : gc_untyped_ptr()
//{}

gc_untyped_ptr::gc_untyped_ptr(byte* ptr)
    : m_handle(ptr)
{
    using namespace threads;
    if (this_managed_thread::is_heap_ptr(&m_handle)) {
        if (this_managed_thread::is_type_meta_requested()) {
            this_managed_thread::register_managed_object_child(&m_handle);
        }
    } else {
        register_root();
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
    delete_root();
}

gc_untyped_ptr& gc_untyped_ptr::operator=(std::nullptr_t t)
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

gc_handle::pin_guard gc_untyped_ptr::untyped_pin() const
{
    return m_handle.pin();
}

gc_handle::stack_pin_guard gc_untyped_ptr::push_untyped_pin() const
{
    return m_handle.push_pin();
}

bool gc_untyped_ptr::is_null() const
{
    return m_handle.is_null();
}

bool gc_untyped_ptr::is_root() const
{
    return gc_is_root(&m_handle);
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
    return gc_tagging::clear(m_handle.rbarrier());
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
    gc_register_root(&m_handle);
}

void gc_untyped_ptr::delete_root()
{
    gc_deregister_root(&m_handle);
}

}}}