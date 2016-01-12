#include "pool_chunk.h"

#include <cassert>

#include "util.h"

namespace precisegc { namespace details {

pool_chunk::pool_chunk(void* chunk, size_t obj_size, std::uint8_t cnt) noexcept
{
    init(chunk, obj_size, cnt);
}

bool pool_chunk::is_memory_available() const noexcept
{
    return m_free.any();
}

void* pool_chunk::allocate(size_t obj_size) noexcept
{
    assert(is_memory_available());
    static_assert(sizeof(size_t) >= sizeof(unsigned long long));
    size_t ind = msb(m_free.to_ullong());
    std::uint8_t* ptr = m_chunk + (ind * obj_size);
    m_free[ind] = false;
    return reinterpret_cast<void*>(ptr);
}

void pool_chunk::deallocate(void* ptr, size_t obj_size) noexcept
{
    std::uint8_t* byte_ptr = reinterpret_cast<std::uint8_t*>(ptr);
    size_t ind = (byte_ptr - m_chunk) / obj_size;
    m_free[ind] = true;
}

void pool_chunk::reset(void* chunk, size_t obj_size, std::uint8_t cnt)
{
    init(chunk, obj_size, cnt);
}

void pool_chunk::init(void* chunk, size_t obj_size, std::uint8_t cnt)
{
    if (!chunk) {
        m_chunk = nullptr;
        m_free.reset();
        return;
    }
    assert(chunk);
    assert(obj_size> 0 && cnt > 0);
    m_chunk = reinterpret_cast<std::uint8_t*>(chunk);
    m_free = std::numeric_limits<size_t>::max() << (CHUNK_MAXSIZE - cnt);
}

}}