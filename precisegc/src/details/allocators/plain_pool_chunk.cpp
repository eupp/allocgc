#include "allocators/plain_pool_chunk.h"

#include <cassert>

namespace precisegc { namespace details { namespace allocators {

plain_pool_chunk::plain_pool_chunk(byte* chunk, size_t size, size_t obj_size) noexcept
    : m_chunk(chunk)
    , m_size(size)
    , m_cnt(size / obj_size)
    , m_bits(calc_bitset(m_cnt))
{
    assert(chunk);
    assert(size > 0 && obj_size > 0 && (size % obj_size == 0));
    assert(m_cnt <= CHUNK_MAXSIZE);
}

byte* plain_pool_chunk::allocate(size_t obj_size) noexcept
{
//    if (memory_available()) {
//        byte* ptr = m_chunk + (m_next * obj_size);
//        m_next = ptr[0];
//        m_available--;
//        return ptr;
//    }
    return nullptr;
}

void plain_pool_chunk::deallocate(byte* ptr, size_t obj_size) noexcept
{
    assert(contains(ptr));
    ptrdiff_t ind = (ptr - m_chunk) / obj_size;
    assert(ind < m_cnt);
    m_bits[ind] = true;
}

bool plain_pool_chunk::contains(byte* ptr) const noexcept
{
    return (m_chunk <= ptr) && (ptr < m_chunk + m_size);
}

bool plain_pool_chunk::memory_available() const noexcept
{
    return m_bits.any();
}

bool plain_pool_chunk::empty(size_t obj_size) const noexcept
{
    return m_bits.count() == m_cnt;
}

byte* plain_pool_chunk::get_mem() const noexcept
{
    return m_chunk;
}

size_t plain_pool_chunk::get_mem_size() const noexcept
{
    return m_size;
}

plain_pool_chunk::bitset_t plain_pool_chunk::calc_bitset(size_t cnt)
{
    return (bitset_t().set()) >> (CHUNK_MAXSIZE - cnt);
}

}}}