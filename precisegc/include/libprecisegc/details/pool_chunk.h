#ifndef DIPLOMA_POOL_CHUNK_H
#define DIPLOMA_POOL_CHUNK_H

#include <cstdint>
#include <cstddef>
#include <limits>

namespace precisegc { namespace details {

class pool_chunk
{
public:

    static const size_t CHUNK_MAXSIZE = std::numeric_limits<std::uint8_t>::max();

    pool_chunk(void* chunk, size_t obj_size, std::uint8_t cnt) noexcept;

    pool_chunk(pool_chunk&&) noexcept = default;

    pool_chunk(const pool_chunk&) = delete;
    pool_chunk& operator=(const pool_chunk&) = delete;
    pool_chunk& operator=(pool_chunk&&) = delete;

    void* allocate(size_t obj_size) noexcept;
    void deallocate(void* ptr, size_t obj_size) noexcept;

    bool is_memory_available() const noexcept;

    void reset(void* chunk, size_t obj_size, std::uint8_t cnt) noexcept;

private:
    void init(void* chunk, size_t obj_size, std::uint8_t cnt) noexcept;

    std::uint8_t* m_chunk;
    std::uint8_t m_next;
    std::uint8_t m_available;
};

}}

#endif //DIPLOMA_POOL_CHUNK_H
