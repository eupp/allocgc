#ifndef DIPLOMA_POOL_UTILITY_H
#define DIPLOMA_POOL_UTILITY_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include <bitset>

#include <libprecisegc/details/types.h>
#include <libprecisegc/details/util.h>
#include <libprecisegc/details/constants.h>
#include <libprecisegc/details/utils/bitset.hpp>

#include <boost/dynamic_bitset.hpp>

namespace precisegc { namespace details { namespace allocators {

class plain_pool_chunk: private noncopyable
{
public:
    static const size_t CHUNK_MAXSIZE = PAGE_SIZE / MIN_CELL_SIZE;

    plain_pool_chunk(byte* chunk, size_t size, size_t obj_size) noexcept;

    plain_pool_chunk(plain_pool_chunk&&) = default;
    plain_pool_chunk& operator=(plain_pool_chunk&&) = default;

    byte* allocate(size_t obj_size) noexcept;
    void  deallocate(byte* ptr, size_t obj_size) noexcept;

    bool contains(byte* ptr) const noexcept;
    bool memory_available() const noexcept;
    bool empty() const noexcept;

    bool is_live(byte* ptr, size_t obj_size) const;
    void set_live(byte* ptr, size_t obj_size, bool live);

    byte* get_mem() const noexcept;
    size_t get_mem_size() const noexcept;

    void swap(plain_pool_chunk& other);
    friend void swap(plain_pool_chunk& a, plain_pool_chunk& b);
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