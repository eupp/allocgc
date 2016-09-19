#include "managed_ptr.hpp"

#include <cassert>
#include <utility>

namespace precisegc { namespace details {

managed_ptr::index_type managed_ptr::indexer;

managed_ptr::managed_ptr() noexcept
    : m_ptr(nullptr)
    , m_descr(nullptr)
{}

managed_ptr::managed_ptr(nullptr_t) noexcept
    : m_ptr(nullptr)
    , m_descr(nullptr)
{}

managed_ptr::managed_ptr(byte* ptr)
    : m_ptr(ptr)
    , m_descr(indexer.get_entry(ptr))
{
    if (m_ptr && !m_descr) {
        throw unindexed_memory_exception(ptr);
    }
}

managed_ptr::managed_ptr(byte* ptr, memory_descriptor* descriptor)
    : m_ptr(ptr)
    , m_descr(descriptor)
{
    assert(indexer.get_entry(ptr) == descriptor || indexer.get_entry(ptr - PAGE_SIZE) == descriptor);
}

bool managed_ptr::get_mark() const
{
    assert(m_descr);
    return m_descr->get_mark(m_ptr);
}

bool managed_ptr::get_pin() const
{
    assert(m_descr);
    return m_descr->get_pin(m_ptr);
}

void managed_ptr::set_mark(bool mark) const
{
    assert(m_descr);
    return m_descr->set_mark(m_ptr, mark);
}

void managed_ptr::set_pin(bool pin) const
{
    assert(m_descr);
    return m_descr->set_pin(m_ptr, pin);
}

size_t managed_ptr::cell_size() const
{
    assert(m_descr);
    return m_descr->cell_size();
}

object_meta* managed_ptr::get_meta() const
{
    assert(m_descr);
    return object_meta::get_meta_ptr(m_descr->get_cell_begin(m_ptr), m_descr->cell_size());
}

byte* managed_ptr::get_cell_begin() const
{
    assert(m_descr);
    return m_descr->get_cell_begin(m_ptr);
}

byte* managed_ptr::get_obj_begin() const
{
    assert(m_descr);
    return object_meta::get_object_ptr(m_descr->get_cell_begin(m_ptr), m_descr->cell_size());
}

byte* managed_ptr::get() const
{
    return m_ptr;
}

memory_descriptor* managed_ptr::get_descriptor() const
{
    return m_descr;
}

void managed_ptr::advance(ptrdiff_t n)
{
    m_ptr += n;
//    assert(get_cell_begin() <= m_ptr && m_ptr <= get_cell_begin() + get_meta()->size());
}

void managed_ptr::swap(managed_ptr& other)
{
    using std::swap;
    swap(m_ptr, other.m_ptr);
    swap(m_descr, other.m_descr);
}

void swap(managed_ptr& a, managed_ptr& b)
{
    a.swap(b);
}

managed_ptr::operator bool() const
{
    return m_ptr != nullptr;
}

managed_ptr managed_ptr::add_to_index(byte* ptr, size_t size, memory_descriptor* descr)
{
    indexer.index(ptr, size, descr);
    return managed_ptr(ptr);
}

void managed_ptr::remove_from_index(byte* ptr, size_t size)
{
    indexer.remove_index(ptr, size);
}

void managed_ptr::remove_from_index(managed_ptr& ptr, size_t size)
{
    indexer.remove_index(ptr.m_ptr, size);
    managed_ptr().swap(ptr);
}

memory_descriptor* managed_ptr::index(byte* ptr)
{
    return indexer.get_entry(ptr);
}

}}