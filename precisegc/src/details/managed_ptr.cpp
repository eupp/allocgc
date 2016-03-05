#include "managed_ptr.h"

namespace precisegc { namespace details {


managed_cell_ptr::managed_cell_ptr()
    : managed_cell_ptr(nullptr)
{}

managed_cell_ptr::managed_cell_ptr(nullptr_t t)
    : pointer_decorator_t(nullptr)
    , m_descr(nullptr)
{}

managed_cell_ptr::managed_cell_ptr(managed_ptr idx_ptr)
    : pointer_decorator_t(idx_ptr)
    , m_descr(nullptr)
{}

managed_cell_ptr::managed_cell_ptr(managed_ptr idx_ptr, managed_memory_descriptor* descriptor,
                                   managed_cell_ptr::lock_type&& lock)
    : pointer_decorator_t(idx_ptr)
    , m_descr(descriptor)
    , m_lock(std::move(lock))
{}

bool managed_cell_ptr::get_mark() const
{
    return false;
}

bool managed_cell_ptr::get_pin() const
{
    return false;
}

void managed_cell_ptr::set_mark(byte* ptr, bool mark)
{

}

void managed_cell_ptr::set_pin(byte* ptr, bool pin)
{

}

void managed_cell_ptr::shade(byte* ptr)
{

}

object_meta* managed_cell_ptr::get_meta() const
{
    return nullptr;
}

bool managed_cell_ptr::owns_descriptor_lock() const
{
    return m_lock.owns_lock();
}

}}