#ifndef DIPLOMA_POOL_UTILITY_H
#define DIPLOMA_POOL_UTILITY_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include "types.h"
#include "../util.h"

namespace precisegc { namespace details { namespace allocators {

class pool_chunk
{
public:
    static const size_t CHUNK_MAXSIZE = std::numeric_limits<byte>::max();

    pool_chunk(byte* chunk, size_t obj_size, byte cnt) noexcept;

    pool_chunk(pool_chunk&&) noexcept = default;

    pool_chunk(const pool_chunk&) = delete;
    pool_chunk& operator=(const pool_chunk&) = delete;
    pool_chunk& operator=(pool_chunk&&) = delete;

    byte* allocate(size_t obj_size) noexcept;
    void deallocate(byte* ptr, size_t obj_size) noexcept;

    bool is_memory_available() const noexcept;
private:
    byte* m_chunk;
    byte m_next;
    byte m_available;
};

template <typename Alloc>
class pool_strip: private noncopyable, private nonmovable
{
public:

    pool_strip();

    byte* allocate(size_t obj_size);
    void deallocate(byte* ptr, size_t obj_size);

private:
    std::vector<pool_chunk, Alloc> m_chunks;
    size_t m_alloc_chunk;
};

}}}

#endif //DIPLOMA_POOL_UTILITY_H
