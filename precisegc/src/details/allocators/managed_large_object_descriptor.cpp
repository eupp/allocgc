#include <libprecisegc/details/allocators/managed_large_object_descriptor.hpp>

#include <cassert>

namespace precisegc { namespace details { namespace allocators {

managed_large_object_descriptor::managed_large_object_descriptor(byte* ptr, size_t size, size_t obj_size)
    : m_ptr(ptr)
    , m_size(size)
    , m_live_bit(true)
    , m_mark_bit(false)
    , m_pin_bit(false)
{
    assert(ptr);
    assert(size > LARGE_CELL_SIZE);
    assert(obj_size <= size);
    managed_ptr::add_to_index(ptr, size, get_descriptor());
}

managed_large_object_descriptor::~managed_large_object_descriptor()
{
    managed_ptr::remove_from_index(m_ptr, m_size);
}

bool managed_large_object_descriptor::get_mark(byte* ptr) const
{
    assert(check_ptr(ptr));
    return m_mark_bit;
}

bool managed_large_object_descriptor::get_pin(byte* ptr) const
{
    assert(check_ptr(ptr));
    return m_pin_bit;
}

void managed_large_object_descriptor::set_mark(byte* ptr, bool mark)
{
    assert(check_ptr(ptr));
    m_mark_bit = mark;
}

void managed_large_object_descriptor::set_pin(byte* ptr, bool pin)
{
    assert(check_ptr(ptr));
    m_pin_bit = pin;
}

bool managed_large_object_descriptor::is_live(byte* ptr) const
{
    assert(check_ptr(ptr));
    return m_live_bit;
}

void managed_large_object_descriptor::set_live(byte* ptr, bool live)
{
    assert(check_ptr(ptr));
    m_live_bit = live;
}

void managed_large_object_descriptor::sweep(byte* ptr)
{
    assert(check_ptr(ptr));
    m_live_bit = false;
}

size_t managed_large_object_descriptor::cell_size() const
{
    return m_size;
}

object_meta* managed_large_object_descriptor::get_cell_meta(byte* ptr) const
{
    assert(check_ptr(ptr));
    return object_meta::get_meta_ptr((void*) m_ptr, m_size);
}

byte* managed_large_object_descriptor::get_cell_begin(byte* ptr) const
{
    assert(check_ptr(ptr));
    return m_ptr;
}

byte * managed_large_object_descriptor::get_obj_begin(byte* ptr) const
{
    object_meta* meta = get_cell_meta(ptr);
    byte* cell_ptr = get_cell_begin(ptr);
    size_t obj_size = meta->get_type_meta()->type_size();
    size_t obj_ind = (ptr - cell_ptr) / obj_size;
    return cell_ptr + obj_ind * obj_size;
}

managed_large_object_descriptor::pointer_type managed_large_object_descriptor::allocate(size_t size)
{
    assert(size <= m_size);
    return utils::make_block_ptr(managed_ptr(m_ptr), m_size);
}

void managed_large_object_descriptor::deallocate(const pointer_type& ptr, size_t size)
{
    assert(ptr.decorated().get() == m_ptr && size == m_size);
    return;
}

bool managed_large_object_descriptor::contains(const pointer_type& ptr) const noexcept
{
    byte* raw_ptr = ptr.decorated().get();
    return m_ptr <= raw_ptr && raw_ptr < m_ptr + m_size;
}

bool managed_large_object_descriptor::memory_available() const noexcept
{
    return false;
}

bool managed_large_object_descriptor::empty() const
{
    return !m_mark_bit && !m_pin_bit;
}

byte* managed_large_object_descriptor::get_mem() const
{
    return m_ptr;
}

size_t managed_large_object_descriptor::get_mem_size() const
{
    return m_size;
}

memory_descriptor* managed_large_object_descriptor::get_descriptor()
{
    return this;
}

managed_large_object_descriptor::iterator managed_large_object_descriptor::begin()
{
    return iterator(m_ptr, get_descriptor());
}

managed_large_object_descriptor::iterator managed_large_object_descriptor::end()
{
    return iterator(m_ptr + m_size, get_descriptor());
}

bool managed_large_object_descriptor::check_ptr(byte* ptr) const
{
    return (m_ptr <= ptr) && (ptr < m_ptr + m_size);
}



}}}
