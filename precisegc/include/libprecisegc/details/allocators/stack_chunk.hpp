#ifndef DIPLOMA_STACK_CHUNK_HPP
#define DIPLOMA_STACK_CHUNK_HPP

#include <cstddef>
#include <limits>

#include <libprecisegc/details/allocators/allocator_tag.hpp>
#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details { namespace allocators {

class stack_chunk
{
public:
    typedef byte* pointer_type;
    typedef allocators::multi_block_chunk_tag chunk_tag;

    static const size_t CHUNK_MAXSIZE = std::numeric_limits<size_t>::max();
    static const size_t DEFAULT_CHUNK_SIZE = 32;

    static size_t chunk_size(size_t cell_size)
    {
        return DEFAULT_CHUNK_SIZE * cell_size;
    }

    stack_chunk();
    stack_chunk(byte* chunk, size_t size, size_t obj_size);

    stack_chunk(stack_chunk&&) = default;
    stack_chunk& operator=(stack_chunk&&) = default;

    byte* allocate(size_t obj_size);
    void  deallocate(byte* ptr, size_t obj_size);

    bool contains(byte* ptr) const;
    bool memory_available() const;
    bool empty() const;

    byte* get_mem() const;
    size_t get_mem_size() const;

    byte* get_top();
private:
    byte*  m_mem;
    byte*  m_top;
    size_t m_size;
};

}}}

#endif //DIPLOMA_STACK_CHUNK_HPP
