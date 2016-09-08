#include <libprecisegc/details/allocators/managed_ptr_iterator.hpp>

namespace precisegc { namespace details { namespace allocators {

managed_ptr_iterator::managed_ptr_iterator() noexcept
{}

managed_ptr_iterator::managed_ptr_iterator(byte* ptr, memory_descriptor* descr) noexcept
        : m_ptr(ptr, descr)
{}

managed_ptr managed_ptr_iterator::dereference() const
{
    return m_ptr;
}

bool managed_ptr_iterator::equal(const managed_ptr_iterator& other) const noexcept
{
    return m_ptr.get() == other.m_ptr.get();
}

void managed_ptr_iterator::increment() noexcept
{
    m_ptr.advance(cell_size());
}

void managed_ptr_iterator::decrement() noexcept
{
    m_ptr.advance(-cell_size());
}

void managed_ptr_iterator::advance(ptrdiff_t n)
{
    m_ptr.advance(n * cell_size());
}

ptrdiff_t managed_ptr_iterator::distance_to(const managed_ptr_iterator& other) const
{
    return other.m_ptr.get() - m_ptr.get();
}

size_t managed_ptr_iterator::cell_size() const
{
    managed_pool_chunk* chunk = static_cast<managed_pool_chunk*>(m_ptr.get_descriptor());
    return chunk->cell_size();
}

}}}

