#ifndef DIPLOMA_POOL_UTILITY_H
#define DIPLOMA_POOL_UTILITY_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include "types.h"
#include "../util.h"

namespace precisegc { namespace details { namespace allocators {

class plain_pool_chunk: private noncopyable
{
public:
    static const size_t CHUNK_MAXSIZE = std::numeric_limits<byte>::max();

    plain_pool_chunk(byte* chunk, size_t size, size_t obj_size) noexcept;

    plain_pool_chunk(plain_pool_chunk&&) noexcept = default;
    plain_pool_chunk& operator=(plain_pool_chunk&&) = default;

    byte* allocate(size_t obj_size) noexcept;
    void deallocate(byte* ptr, size_t obj_size) noexcept;

    bool contains(byte* ptr) const noexcept;
    bool memory_available() const noexcept;
    bool empty(size_t obj_size) const noexcept;
private:
    byte* m_chunk;
    size_t m_size;
    byte m_next;
    byte m_available;
};

}}}

#endif //DIPLOMA_POOL_UTILITY_H