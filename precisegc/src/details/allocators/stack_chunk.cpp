#include <libprecisegc/details/allocators/stack_chunk.hpp>

#include <cassert>
#include <cstring>

namespace precisegc { namespace details { namespace allocators {

stack_chunk::stack_chunk()
    : m_mem(nullptr)
    , m_mem_end(nullptr)
    , m_top(nullptr)
    , m_next(nullptr)
{}

stack_chunk::stack_chunk(byte* chunk, size_t size, size_t obj_size)
    : m_mem(chunk)
    , m_mem_end(chunk + size)
    , m_top(chunk)
    , m_next(chunk)
{
    assert(chunk);
    assert(size > 0 && obj_size > 0);
}

byte* stack_chunk::allocate(size_t obj_size)
{
    assert(m_mem);
    assert(memory_available());
    if (m_next == m_top) {
        byte* ptr = m_top;
        m_top  += obj_size;
        m_next += obj_size;
        return ptr;
    }
    byte* ptr = m_next;
    m_next = *reinterpret_cast<byte**>(m_next);
    // a bit of bad design: in general we doesn't have to zero allocated memory,
    // but since we use instances of this class only inside gc_heap we do it here
    // (because it's only place where we can distinct stack-allocated memory from freelist-allocated
    memset(ptr, 0, obj_size);
    return ptr;
}

void stack_chunk::deallocate(byte* ptr, size_t obj_size)
{
    assert(m_mem);
    assert(contains(ptr));
    byte** next = reinterpret_cast<byte**>(ptr);
    *next = m_next;
    m_next = ptr;
}

byte* stack_chunk::get_mem() const
{
    return m_mem;
}

size_t stack_chunk::get_mem_size() const
{
    return m_mem_end - m_mem;
}

byte* stack_chunk::get_top()
{
    return m_top;
}

void stack_chunk::reset()
{
    m_next = m_top;
}

}}}
