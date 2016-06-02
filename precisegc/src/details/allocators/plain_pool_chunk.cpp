#include "allocators/plain_pool_chunk.h"

#include <cassert>
#include <utility>

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
    auto idx_opt = m_bits.least_significant_bit();
    if (idx_opt) {
        size_t idx = *idx_opt;
        m_bits.reset(idx);
        return get_mem() + idx * obj_size;
    }
    return nullptr;
}

void plain_pool_chunk::deallocate(byte* ptr, size_t obj_size) noexcept
{
    set_live(ptr, obj_size, false);
}

bool plain_pool_chunk::contains(byte* ptr) const noexcept
{
    return (m_chunk <= ptr) && (ptr < m_chunk + m_size);
}

bool plain_pool_chunk::memory_available() const noexcept
{
    return m_bits.any();
}

bool plain_pool_chunk::empty() const noexcept
{
    return m_bits.count() == m_cnt;
}

bool plain_pool_chunk::is_live(byte* ptr, size_t obj_size) const
{
    assert(contains(ptr));
    ptrdiff_t ind = (ptr - m_chunk) / obj_size;
    assert(ind < m_cnt);
    return !(m_bits.get(ind));
}

void plain_pool_chunk::set_live(byte* ptr, size_t obj_size, bool live)
{
    assert(contains(ptr));
    ptrdiff_t ind = (ptr - m_chunk) / obj_size;
    assert(ind < m_cnt);
    m_bits.set(ind, !live);
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
    return (bitset_t().set_all()) >> (CHUNK_MAXSIZE - cnt);
}

void plain_pool_chunk::swap(plain_pool_chunk& other)
{
    using std::swap;
    swap(m_chunk, other.m_chunk);
    swap(m_size, other.m_size);
    swap(m_cnt, other.m_cnt);
    swap(m_bits, other.m_bits);
}

void swap(plain_pool_chunk& a, plain_pool_chunk& b)
{
    a.swap(b);
}

}}}