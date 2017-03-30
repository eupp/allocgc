#include <liballocgc/details/allocators/freelist_pool_chunk.hpp>

#include <cassert>
#include <cstring>
#include <algorithm>

namespace allocgc { namespace details { namespace allocators {

freelist_pool_chunk::freelist_pool_chunk()
    : m_chunk(nullptr)
    , m_size(0)
    , m_next(nullptr)
    , m_init(nullptr)
    , m_gain(0)
    , m_alloc_cnt(0)
{}

freelist_pool_chunk::freelist_pool_chunk(byte* chunk, size_t size, size_t obj_size)
    : m_chunk(chunk)
    , m_size(size)
    , m_next(chunk)
    , m_init(chunk)
    , m_gain(1)
    , m_alloc_cnt(0)
{
    assert(chunk);
    assert(size > 0 && obj_size > 0 && (size % obj_size == 0));
    assert(obj_size != 0 && obj_size % sizeof(byte*) == 0);
}

byte* freelist_pool_chunk::allocate(size_t obj_size)
{
    assert(memory_available());
    if (m_next == m_init) {
        byte* init_end = std::min(m_init + obj_size * m_gain, m_chunk + m_size);
        for (byte* it = m_init; it < init_end; it += obj_size) {
            byte* next = it + obj_size;
            memcpy(it, &next, sizeof(byte*));
        }
        m_init = init_end;
        m_gain *= 2;
    }
    byte* p = m_next;
    m_next = *reinterpret_cast<byte**>(m_next);
    ++m_alloc_cnt;
    return p;
}

void freelist_pool_chunk::deallocate(byte* ptr, size_t obj_size)
{
    assert(ptr /*&& reinterpret_cast<std::uintptr_t>(ptr) % obj_size == 0*/);
    memcpy(ptr, &m_next, sizeof(byte*));
    m_next = ptr;
    --m_alloc_cnt;
}

bool freelist_pool_chunk::contains(byte* ptr) const
{
    return (m_chunk <= ptr) && (ptr < m_chunk + m_size);
}

bool freelist_pool_chunk::memory_available() const
{
    return m_next != m_chunk + m_size;
}

bool freelist_pool_chunk::empty() const
{
    return m_alloc_cnt == 0;
}

byte* freelist_pool_chunk::memory() const
{
    return m_chunk;
}

size_t freelist_pool_chunk::size() const
{
    return m_size;
}

}}}