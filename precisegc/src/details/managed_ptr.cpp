#include "managed_ptr.h"

namespace precisegc { namespace details {


managed_cell_ptr::managed_cell_ptr()
    : managed_cell_ptr(nullptr)
{}

managed_cell_ptr::managed_cell_ptr(nullptr_t t)
    : m_ptr(nullptr)
    , m_descr(nullptr)
{}

managed_cell_ptr::managed_cell_ptr(managed_ptr idx_ptr)
    : m_ptr(idx_ptr)
    , m_descr(nullptr)
{}

managed_cell_ptr::managed_cell_ptr(managed_ptr idx_ptr, managed_memory_descriptor* descriptor)
    : m_ptr(idx_ptr)
    , m_descr(descriptor)
{}

managed_cell_ptr::managed_cell_ptr(managed_ptr idx_ptr, managed_memory_descriptor* descriptor,
                                   managed_cell_ptr::lock_type&& lock)
    : m_ptr(idx_ptr)
    , m_descr(descriptor)
    , m_lock(std::move(lock))
{}

bool managed_cell_ptr::get_mark() const
{
    descriptor_lazy_init();
    return m_descr->get_mark(get());
}

bool managed_cell_ptr::get_pin() const
{
    descriptor_lazy_init();
    return m_descr->get_pin(get());
}

void managed_cell_ptr::set_mark(bool mark)
{
    descriptor_lazy_init();
    return m_descr->set_mark(get(), mark);
}

void managed_cell_ptr::set_pin(bool pin)
{
    descriptor_lazy_init();
    return m_descr->set_pin(get(), pin);
}

void managed_cell_ptr::sweep()
{
    descriptor_lazy_init();
    m_descr->sweep(get());
}

object_meta* managed_cell_ptr::get_meta() const
{
    descriptor_lazy_init();
    return m_descr->get_cell_meta(get());
}

byte* managed_cell_ptr::get() const
{
    return m_ptr.get();
}

bool managed_cell_ptr::owns_descriptor_lock() const
{
    return m_lock.owns_lock();
}

void managed_cell_ptr::descriptor_lazy_init() const
{
    if (!m_descr) {
        m_descr = m_ptr.get_indexed_entry();
    }
    if (!m_descr) {
        throw unindexed_memory_exception();
    }
}

const char* managed_cell_ptr::unindexed_memory_exception::what() const noexcept
{
    return "Inappropriate access to unindexed memory";
}
}}