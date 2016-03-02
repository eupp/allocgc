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

    plain_pool_chunk() noexcept;
    plain_pool_chunk(byte* chunk, size_t size, size_t obj_size) noexcept;

    plain_pool_chunk(plain_pool_chunk&&) noexcept = default;
    plain_pool_chunk& operator=(plain_pool_chunk&&) = default;

    byte* allocate(size_t obj_size) noexcept;
    void deallocate(byte* ptr, size_t obj_size) noexcept;

    bool contains(byte* ptr) const noexcept;
    bool memory_available() const noexcept;
    bool empty(size_t obj_size) const noexcept;

    byte* get_mem() const noexcept;
    size_t get_mem_size() const noexcept;

    template <typename Alloc>
    static plain_pool_chunk create(size_t obj_size, Alloc& allocator)
    {
        size_t chunk_size = CHUNK_MAXSIZE * obj_size;
        return plain_pool_chunk(allocator.allocate(chunk_size), chunk_size, obj_size);
    }

    template <typename Alloc>
    static void destroy(plain_pool_chunk& chk, size_t obj_size, Alloc& allocator)
    {
        allocator.deallocate(chk.m_chunk, chk.m_size);
        plain_pool_chunk().swap(chk);
    }

    void swap(plain_pool_chunk& other)
    {
        using std::swap;
        swap(m_chunk, other.m_chunk);
        swap(m_size, other.m_size);
        swap(m_next, other.m_next);
        swap(m_available, other.m_available);
    }

    friend void swap(plain_pool_chunk& a, plain_pool_chunk& b)
    {
        a.swap(b);
    }
private:
    byte* m_chunk;
    size_t m_size;
    byte m_next;
    byte m_available;
};

}}}

#endif //DIPLOMA_POOL_UTILITY_H