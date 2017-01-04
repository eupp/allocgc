#ifndef DIPLOMA_FREELIST_POOL_CHUNK_HPP
#define DIPLOMA_FREELIST_POOL_CHUNK_HPP

#include <cstddef>
#include <limits>

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/allocators/allocator_tag.hpp>

namespace precisegc { namespace details { namespace allocators {

class freelist_pool_chunk : private utils::noncopyable, private utils::nonmovable
{
public:
    typedef byte* pointer_type;

    static const size_t CHUNK_MAXSIZE = std::numeric_limits<size_t>::max();
    static const size_t DEFAULT_CHUNK_SIZE = 32;

    static size_t chunk_size(size_t cell_size)
    {
        return DEFAULT_CHUNK_SIZE * cell_size;
    }

    freelist_pool_chunk();
    freelist_pool_chunk(byte* chunk, size_t size, size_t obj_size);

    freelist_pool_chunk(freelist_pool_chunk&&) = default;
    freelist_pool_chunk& operator=(freelist_pool_chunk&&) = default;

    byte* allocate(size_t obj_size);
    void  deallocate(byte* ptr, size_t obj_size);

    bool contains(byte* ptr) const;
    bool memory_available() const;
    bool empty() const;

    byte*  memory() const;
    size_t size() const;
private:
    byte*  m_chunk;
    size_t m_size;
    byte*  m_next;
    byte*  m_init;
    size_t m_gain;
    size_t m_alloc_cnt;
};

}}}

#endif //DIPLOMA_FREELIST_POOL_CHUNK_HPP
