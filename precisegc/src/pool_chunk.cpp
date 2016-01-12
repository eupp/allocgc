#include "pool_chunk.h"

#include <cassert>

namespace precisegc { namespace details {

pool_chunk::pool_chunk(void* chunk, size_t obj_size, std::uint8_t cnt) noexcept
    : m_chunk(reinterpret_cast<std::uint8_t*>(chunk))
    , m_next(0)
    , m_available(cnt)
{
    assert(chunk);
    assert(obj_size> 0 && cnt > 0);
    std::uint8_t* ptr = m_chunk;
    for (int i = 1; i <= cnt; ++i, ptr += obj_size) {
        ptr[0] = i;
    }
}

bool pool_chunk::is_memory_available() const noexcept
{
    return m_available > 0;
}

void* pool_chunk::allocate(size_t obj_size) noexcept
{
    assert(is_memory_available());
    std::uint8_t* ptr = m_chunk + (m_next * obj_size);
    m_next = ptr[0];
    m_available--;
    return reinterpret_cast<void*>(ptr);
}

void pool_chunk::deallocate(void* ptr, size_t obj_size) noexcept
{
    std::uint8_t* byte_ptr = reinterpret_cast<std::uint8_t*>(ptr);
    byte_ptr[0] = m_next;
    m_next = (byte_ptr - m_chunk) / obj_size;
    m_available++;
}

}}