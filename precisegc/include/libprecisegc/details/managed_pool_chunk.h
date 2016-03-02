#ifndef DIPLOMA_MANAGED_POOL_CHUNK_H
#define DIPLOMA_MANAGED_POOL_CHUNK_H

#include <bitset>
#include <cstdint>
#include <memory>

#include "allocators/plain_pool_chunk.h"
#include "allocators/constants.h"
#include "managed_memory_descriptor.h"
#include "managed_ptr.h"
#include "mutex.h"
#include "util.h"

namespace precisegc { namespace details {

class managed_pool_chunk : private noncopyable
{
    typedef std::uintptr_t uintptr;
    typedef allocators::plain_pool_chunk plain_pool_chunk;
    typedef managed_memory_descriptor::lock_type lock_type;
public:
    typedef ptr_manager pointer_type;

    static const size_t CHUNK_MAXSIZE = PAGE_SIZE / MIN_CELL_SIZE;
    static const size_t CHUNK_MINSIZE = 32;

    class memory_descriptor : public managed_memory_descriptor
    {
        typedef std::bitset<CHUNK_MAXSIZE> bitset_t;
    public:
        memory_descriptor(managed_pool_chunk* chunk_ptr, byte* chunk_mem, size_t size, size_t cell_size);
        ~memory_descriptor();

        managed_pool_chunk* get_pool_chunk() const;
        void set_pool_chunk(managed_pool_chunk* pool_chunk);
    private:
        static uintptr calculate_mask(byte* chunk, size_t chunk_size, size_t cell_size);

        managed_pool_chunk* m_chunk_ptr;
        size_t m_cell_size;
        uintptr m_mask;
        mutex m_mutex;
        bitset_t m_mark_bits;
        bitset_t m_pin_bits;
    };

    managed_pool_chunk();
    managed_pool_chunk(byte* chunk, size_t size, size_t cell_size, memory_descriptor* descr);

    managed_pool_chunk(managed_pool_chunk&&);
    managed_pool_chunk& operator=(managed_pool_chunk&&) = delete;

    ~managed_pool_chunk();

    ptr_manager allocate(size_t cell_size);
    void deallocate(ptr_manager ptr, size_t cell_size);

    bool contains(byte* ptr) const noexcept;
    bool memory_available() const noexcept;
    bool empty(size_t cell_size) const noexcept;

    template <typename Alloc>
    static managed_pool_chunk create(size_t cell_size, Alloc& allocator)
    {
        assert(PAGE_SIZE % cell_size == 0);
        size_t chunk_size = std::max(CHUNK_MINSIZE, PAGE_SIZE / cell_size);
        byte* raw_ptr = allocator.allocate(chunk_size);

        memory_descriptor* descr = new memory_descriptor(nullptr, raw_ptr, chunk_size, cell_size);
        managed_pool_chunk chunk = managed_pool_chunk(raw_ptr, chunk_size, cell_size, descr);
        descr->set_pool_chunk(&chunk);
        return chunk;
    }

    template <typename Alloc>
    static void destroy(managed_pool_chunk& chk, size_t obj_size, Alloc& allocator)
    {
        delete chk.m_descr;
        allocator.deallocate(chk.m_chunk.get_mem(), chk.m_chunk.get_mem_size());
        managed_pool_chunk().swap(chk);
    }

private:
    void swap(managed_pool_chunk& other);

    plain_pool_chunk m_chunk;
    memory_descriptor* m_descr;
};

}}

#endif //DIPLOMA_MANAGED_POOL_CHUNK_H
