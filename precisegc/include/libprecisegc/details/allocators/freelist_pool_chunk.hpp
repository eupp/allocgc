#ifndef DIPLOMA_FREELIST_POOL_CHUNK_HPP
#define DIPLOMA_FREELIST_POOL_CHUNK_HPP

#include <cstddef>
#include <limits>

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/constants.hpp>

namespace precisegc { namespace details { namespace allocators {

class freelist_pool_chunk : private utils::noncopyable, private utils::nonmovable
{
public:
    static const size_t CHUNK_MAXSIZE = std::numeric_limits<size_t>::max();

    freelist_pool_chunk(byte* chunk, size_t size, size_t obj_size);

    byte* allocate(size_t obj_size);
    void  deallocate(byte* ptr, size_t obj_size);

    bool contains(byte* ptr) const;
    bool memory_available() const;
    bool empty() const;

    byte* get_mem() const;
    size_t get_mem_size() const;
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
