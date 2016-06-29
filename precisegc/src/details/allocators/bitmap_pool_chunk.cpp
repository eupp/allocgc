#include "allocators/bitmap_pool_chunk.hpp"

#include <cassert>
#include <utility>

namespace precisegc { namespace details { namespace allocators {

bitmap_pool_chunk::bitmap_pool_chunk(byte* chunk, size_t size, size_t obj_size) 
    : m_chunk(chunk)
    , m_size(size)
    , m_cnt(size / obj_size)
    , m_bits(calc_bitset(m_cnt))
{
    assert(chunk);
    assert(size > 0 && obj_size > 0 && (size % obj_size == 0));
    assert(m_cnt <= CHUNK_MAXSIZE);
}

byte* bitmap_pool_chunk::allocate(size_t obj_size) 
{
    assert(memory_available());
    auto idx_opt = m_bits.least_significant_bit();
    size_t idx = *idx_opt;
    m_bits.reset(idx);
    return get_mem() + idx * obj_size;
}

void bitmap_pool_chunk::deallocate(byte* ptr, size_t obj_size) 
{
    set_live(ptr, obj_size, false);
}

bool bitmap_pool_chunk::contains(byte* ptr) const 
{
    return (m_chunk <= ptr) && (ptr < m_chunk + m_size);
}

bool bitmap_pool_chunk::memory_available() const 
{
    return m_bits.any();
}

bool bitmap_pool_chunk::empty() const 
{
    return m_bits.count() == m_cnt;
}

bool bitmap_pool_chunk::is_live(byte* ptr, size_t obj_size) const
{
    assert(contains(ptr));
    ptrdiff_t ind = (ptr - m_chunk) / obj_size;
    assert(ind < m_cnt);
    return !(m_bits.get(ind));
}

void bitmap_pool_chunk::set_live(byte* ptr, size_t obj_size, bool live)
{
    assert(contains(ptr));
    ptrdiff_t ind = (ptr - m_chunk) / obj_size;
    assert(ind < m_cnt);
    m_bits.set(ind, !live);
}

byte* bitmap_pool_chunk::get_mem() const 
{
    return m_chunk;
}

size_t bitmap_pool_chunk::get_mem_size() const 
{
    return m_size;
}

bitmap_pool_chunk::bitset_t bitmap_pool_chunk::calc_bitset(size_t cnt)
{
    return (bitset_t().set_all()) >> (CHUNK_MAXSIZE - cnt);
}

void bitmap_pool_chunk::swap(bitmap_pool_chunk& other)
{
    using std::swap;
    swap(m_chunk, other.m_chunk);
    swap(m_size, other.m_size);
    swap(m_cnt, other.m_cnt);
    swap(m_bits, other.m_bits);
}

void swap(bitmap_pool_chunk& a, bitmap_pool_chunk& b)
{
    a.swap(b);
}

}}}