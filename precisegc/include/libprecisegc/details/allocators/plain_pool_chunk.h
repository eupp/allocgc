#ifndef DIPLOMA_POOL_UTILITY_H
#define DIPLOMA_POOL_UTILITY_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include "types.h"
#include "../util.h"

namespace precisegc { namespace details { namespace allocators {

class plain_pool_chunk
{
public:
    static const size_t CHUNK_MAXSIZE = std::numeric_limits<byte>::max();

    plain_pool_chunk(byte* chunk, size_t size, size_t obj_size) noexcept;

    plain_pool_chunk(plain_pool_chunk&&) noexcept = default;

    plain_pool_chunk(const plain_pool_chunk&) = delete;
    plain_pool_chunk& operator=(const plain_pool_chunk&) = delete;
    plain_pool_chunk& operator=(plain_pool_chunk&&) = delete;

    byte* allocate(size_t obj_size) noexcept;
    void deallocate(byte* ptr, size_t obj_size) noexcept;

    bool is_memory_available() const noexcept;
private:
    byte* m_chunk;
    byte m_next;
    byte m_available;
};

}}}

#endif //DIPLOMA_POOL_UTILITY_H
