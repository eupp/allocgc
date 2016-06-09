#ifndef DIPLOMA_POOL_UTILITY_H
#define DIPLOMA_POOL_UTILITY_H

#include <cstddef>

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/utils/bitset.hpp>

namespace precisegc { namespace details { namespace allocators {

class bitmap_pool_chunk: private utils::noncopyable, private utils::nonmovable
{
public:
    static const size_t CHUNK_MAXSIZE = PAGE_SIZE / MIN_CELL_SIZE;

    bitmap_pool_chunk(byte* chunk, size_t size, size_t obj_size);

    byte* allocate(size_t obj_size);
    void  deallocate(byte* ptr, size_t obj_size);

    bool contains(byte* ptr) const;
    bool memory_available() const;
    bool empty() const;

    bool is_live(byte* ptr, size_t obj_size) const;
    void set_live(byte* ptr, size_t obj_size, bool live);

    byte* get_mem() const;
    size_t get_mem_size() const;

    void swap(bitmap_pool_chunk& other);
    friend void swap(bitmap_pool_chunk& a, bitmap_pool_chunk& b);
private:
    typedef utils::bitset<CHUNK_MAXSIZE> bitset_t;

    // returns bitmap suitable for storing bits for cnt objects
    static bitset_t calc_bitset(size_t cnt);

    byte*  m_chunk;
    size_t m_size;
    size_t m_cnt;
    bitset_t m_bits;
};

}}}

#endif //DIPLOMA_POOL_UTILITY_H