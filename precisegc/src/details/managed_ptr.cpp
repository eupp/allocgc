#include "managed_ptr.hpp"

#include <cassert>
#include <utility>

namespace precisegc { namespace details {

index_type managed_ptr::indexer{};

managed_ptr::managed_ptr()
    : m_ptr(nullptr)
    , m_descr(nullptr)
{}

managed_ptr::managed_ptr(byte* ptr)
    : m_ptr(ptr)
    , m_descr(indexer.get_entry(ptr))
{
    if (!m_descr) {
        throw unindexed_memory_exception(ptr);
    }
}

managed_ptr::managed_ptr(byte* ptr, managed_memory_descriptor* descriptor)
    : m_ptr(ptr)
    , m_descr(descriptor)
{
    assert(indexer.get_entry(ptr) == descriptor);
}

bool managed_ptr::get_mark() const
{
    return m_descr->get_mark(get());
}

bool managed_ptr::get_pin() const
{
    return m_descr->get_pin(get());
}

void managed_ptr::set_mark(bool mark) const
{
    return m_descr->set_mark(get(), mark);
}

void managed_ptr::set_pin(bool pin) const
{
    return m_descr->set_pin(get(), pin);
}

bool managed_ptr::is_live() const
{
    return m_descr->is_live(get());
}

void managed_ptr::sweep() const
{
    m_descr->sweep(get());
}

object_meta* managed_ptr::get_meta() const
{
    return m_descr->get_cell_meta(get());
}

byte* managed_ptr::get_cell_begin() const
{
    return m_descr->get_cell_begin(get());
}

byte* managed_ptr::get_obj_begin() const
{
    return m_descr->get_obj_begin(get());
}

byte* managed_ptr::get() const
{
    return m_ptr;
}

managed_memory_descriptor* managed_ptr::get_descriptor() const
{
    return m_descr;
}

void managed_ptr::advance(ptrdiff_t n)
{
    m_ptr += n;
    assert(get_cell_begin() <= m_ptr && m_ptr <= get_cell_begin() + get_meta()->size());
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

managed_ptr managed_ptr::add_to_index(byte* ptr, size_t size, managed_memory_descriptor* descr)
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
    indexer.remove_index(ptr.get(), size);
    managed_ptr().swap(ptr);
}

managed_memory_descriptor* managed_ptr::index(byte* ptr)
{
    return indexer.get_entry(ptr);
}

}}