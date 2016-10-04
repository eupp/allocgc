#include <libprecisegc/details/allocators/managed_object_descriptor.hpp>

#include <cassert>

namespace precisegc { namespace details { namespace allocators {

managed_object_descriptor::managed_object_descriptor()
    : m_ptr(nullptr)
    , m_size(0)
    , m_mark_bit(false)
    , m_pin_bit(false)
{}

managed_object_descriptor::managed_object_descriptor(byte* ptr, size_t size, size_t obj_size)
    : m_ptr(ptr)
    , m_size(size)
    , m_mark_bit(false)
    , m_pin_bit(false)
{
    assert(ptr);
    assert(size > LARGE_CELL_SIZE);
    assert(obj_size <= size);
    indexed_managed_object::add_to_index(ptr, size, this);
}

managed_object_descriptor::~managed_object_descriptor()
{
    indexed_managed_object::remove_from_index(m_ptr, m_size);
}

bool managed_object_descriptor::get_mark(byte* ptr) const
{
    assert(check_ptr(ptr));
    return m_mark_bit;
}

bool managed_object_descriptor::get_pin(byte* ptr) const
{
    assert(check_ptr(ptr));
    return m_pin_bit;
}

void managed_object_descriptor::set_mark(byte* ptr, bool mark)
{
    assert(check_ptr(ptr));
    m_mark_bit = mark;
}

void managed_object_descriptor::set_pin(byte* ptr, bool pin)
{
    assert(check_ptr(ptr));
    m_pin_bit = pin;
}

size_t managed_object_descriptor::cell_size() const
{
    return m_size;
}

byte* managed_object_descriptor::cell_start(byte* ptr) const
{
    assert(check_ptr(ptr));
    return m_ptr;
}

managed_object_descriptor::pointer_type managed_object_descriptor::allocate(size_t size)
{
    assert(size <= m_size);
    return indexed_managed_object(m_ptr, this);
}

void managed_object_descriptor::deallocate(const pointer_type& ptr, size_t size)
{
    assert(ptr.get() == m_ptr && size == m_size);
    return;
}

bool managed_object_descriptor::contains(const pointer_type& ptr) const noexcept
{
    byte* raw_ptr = ptr.get();
    return m_ptr <= raw_ptr && raw_ptr < m_ptr + m_size;
}

bool managed_object_descriptor::memory_available() const noexcept
{
    return false;
}

bool managed_object_descriptor::empty() const
{
    return !m_mark_bit && !m_pin_bit;
}

byte* managed_object_descriptor::get_mem() const
{
    return m_ptr;
}

size_t managed_object_descriptor::get_mem_size() const
{
    return m_size;
}

memory_descriptor* managed_object_descriptor::get_descriptor()
{
    return this;
}

managed_object_descriptor::iterator managed_object_descriptor::begin()
{
    return iterator(m_ptr, this);
}

managed_object_descriptor::iterator managed_object_descriptor::end()
{
    return iterator(m_ptr + m_size, this);
}

bool managed_object_descriptor::check_ptr(byte* ptr) const
{
    return (m_ptr <= ptr) && (ptr < m_ptr + m_size);
}



}}}
