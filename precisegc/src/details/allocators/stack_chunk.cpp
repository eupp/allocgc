#include <libprecisegc/details/allocators/stack_chunk.hpp>

#include <cassert>

namespace precisegc { namespace details { namespace allocators {

stack_chunk::stack_chunk()
    : m_mem(nullptr)
    , m_top(nullptr)
    , m_size(0)
{}

stack_chunk::stack_chunk(byte* chunk, size_t size, size_t obj_size)
    : m_mem(chunk)
    , m_top(m_mem)
    , m_size(size)
{
    assert(chunk);
    assert(size > 0 && obj_size > 0);
}

byte* stack_chunk::allocate(size_t obj_size)
{
    assert(m_mem);
    assert(m_top <= m_mem + m_size - obj_size);
    byte* ptr = m_top;
    m_top += obj_size;
    return ptr;
}

void stack_chunk::deallocate(byte* ptr, size_t obj_size)
{
    assert(m_mem);
    assert(m_top == ptr + obj_size);
    m_top = ptr;
}

bool stack_chunk::contains(byte* ptr) const
{
    return (m_mem <= ptr) && (ptr < m_mem + m_size);
}

bool stack_chunk::memory_available() const
{
    return m_top < m_mem + m_size;
}

bool stack_chunk::empty() const
{
    return m_top == m_mem;
}

byte* stack_chunk::get_mem() const
{
    return m_mem;
}

size_t stack_chunk::get_mem_size() const
{
    return m_size;
}

}}}
